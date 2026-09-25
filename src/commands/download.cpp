#include "commands/download.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include "core/text_util.h"
#include "telegram/message_format.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

constexpr std::size_t kDownloadParallel = 3;
constexpr std::chrono::minutes kDownloadStallTimeout(10);
constexpr std::chrono::seconds kMessageTimeout(30);
constexpr std::chrono::seconds kLinkTimeout(60);
constexpr char kOutputDirectory[] = "downloads";

struct Failure {
  std::string source;
  std::string reason;
};

// 一条待下载视频：解析结果 + 规划出的目标路径 + 下载进度。
struct DownloadTask {
  VideoFile video;
  std::int64_t message_id = 0;
  std::string source;
  std::filesystem::path destination;
  std::int64_t downloaded_size = -1;
  int last_reported_percent = 0;
  std::chrono::steady_clock::time_point last_progress_time;
};

// 一条待解析的消息来源：消息链接，或 chat_id + message_id。
struct MessageRef {
  bool is_link = false;
  std::string link;
  std::int64_t chat_id = 0;
  std::int64_t message_id = 0;
};

std::optional<VideoFile> SelectBestQualityForMessage(
    const td_api::message& message) {
  std::vector<AlternativeVideo> alternatives;
  std::optional<VideoFile> video = ExtractVideoFile(message, &alternatives);
  if (!video || alternatives.empty()) {
    return video;
  }
  VideoFile best = SelectBestQuality(*video, alternatives);
  if (best.file_id == video->file_id) {
    return video;
  }
  std::cerr << "该视频有 " << alternatives.size()
            << " 个备选清晰度，选用最清晰的 " << best.height
            << "p 版本（file_id=" << best.file_id << "）\n";
  return best;
}

bool PathExists(const std::filesystem::path& path) {
  std::error_code error;
  return std::filesystem::exists(path, error) && !error;
}

bool SaveDownloadedFile(const std::filesystem::path& source,
                        const std::filesystem::path& destination,
                        std::string* error) {
  if (!destination.parent_path().empty()) {
    std::error_code create_error;
    std::filesystem::create_directories(destination.parent_path(),
                                        create_error);
    if (create_error) {
      return SetError(error, "创建目录失败：" +
                                 destination.parent_path().string() + "：" +
                                 create_error.message());
    }
  }
  std::error_code copy_error;
  std::filesystem::copy_file(source, destination,
                             std::filesystem::copy_options::overwrite_existing,
                             copy_error);
  if (copy_error) {
    return SetError(error, "复制文件失败：" + source.string() + " -> " +
                               destination.string() + "：" +
                               copy_error.message());
  }
  return true;
}

std::filesystem::path MakeNumberedPath(const std::filesystem::path& path,
                                       int suffix) {
  const std::filesystem::path parent = path.parent_path();
  const std::string stem = path.stem().string();
  const std::string extension = path.extension().string();
  return parent / (stem + "-" + std::to_string(suffix) + extension);
}

std::filesystem::path MakeUniqueBatchDestination(
    const std::filesystem::path& destination,
    std::set<std::filesystem::path>* reserved_destinations) {
  std::filesystem::path candidate = destination;
  int suffix = 1;
  while (PathExists(candidate) ||
         (reserved_destinations != nullptr &&
          reserved_destinations->count(candidate) > 0)) {
    candidate = MakeNumberedPath(destination, suffix);
    ++suffix;
  }
  if (reserved_destinations != nullptr) {
    reserved_destinations->insert(candidate);
  }
  return candidate;
}

bool ReadLinksFromFile(const std::string& path, std::vector<MessageRef>* refs,
                       std::string* error) {
  std::ifstream input(path);
  if (!input.is_open()) {
    return SetError(error, "无法打开链接文件：" + path);
  }

  std::string line;
  while (std::getline(input, line)) {
    const std::string link = Trim(line);
    if (link.empty() || link.rfind("#", 0) == 0) {
      continue;
    }
    refs->push_back(MessageRef{true, link});
  }
  if (input.bad()) {
    return SetError(error, "读取链接文件失败：" + path);
  }
  return true;
}

std::string RefSource(const MessageRef& ref) {
  if (ref.is_link) {
    return ref.link;
  }
  return "chat_id=" + std::to_string(ref.chat_id) +
         " message_id=" + std::to_string(ref.message_id);
}

bool ResolveMessageLink(TelegramClient* client, const std::string& link,
                        td_api::object_ptr<td_api::message>* message,
                        std::string* reason) {
  std::string request_error;
  auto info_object =
      client->Request(td_api::make_object<td_api::getMessageLinkInfo>(link),
                      kLinkTimeout, &request_error);
  if (!info_object) {
    return SetError(reason, "无法解析该链接：" + OneLine(request_error));
  }

  auto info =
      td::move_tl_object_as<td_api::messageLinkInfo>(std::move(info_object));
  if (!info->message_) {
    return SetError(reason,
                    "拿不到消息内容，可能需要先加入"
                    "该群/频道（chat_id=" +
                        std::to_string(info->chat_id_) + "）");
  }
  *message = std::move(info->message_);
  return true;
}

bool ResolveRef(TelegramClient* client, const MessageRef& ref,
                DownloadTask* resolved, std::string* reason) {
  const std::string source = RefSource(ref);
  td_api::object_ptr<td_api::message> message;

  if (ref.is_link) {
    if (!ResolveMessageLink(client, ref.link, &message, reason)) {
      return false;
    }
  } else {
    if (!client->LoadAllChats(reason)) {
      return false;
    }
    std::string request_error;
    auto message_object = client->Request(
        td_api::make_object<td_api::getMessage>(ref.chat_id, ref.message_id),
        kMessageTimeout, &request_error);
    if (!message_object) {
      return SetError(reason,
                      "无法读取 " + source + "：" + OneLine(request_error));
    }
    message = td::move_tl_object_as<td_api::message>(std::move(message_object));
  }

  std::optional<VideoFile> video = SelectBestQualityForMessage(*message);
  if (!video) {
    return SetError(reason, source + " 的消息里没有可下载的视频");
  }
  resolved->video = *video;
  resolved->message_id = message->id_;
  resolved->source = source;
  return true;
}

bool PlanTasks(std::vector<DownloadTask>* tasks, std::string* error) {
  std::map<std::int32_t, std::vector<std::int64_t>> message_ids_by_file_id;
  for (const DownloadTask& task : *tasks) {
    message_ids_by_file_id[task.video.file_id].push_back(task.message_id);
  }

  bool has_duplicates = false;
  for (const auto& item : message_ids_by_file_id) {
    if (item.second.size() <= 1) {
      continue;
    }
    has_duplicates = true;
    std::cerr << "重复视频：file_id=" << item.first << " message_id=";
    for (std::size_t index = 0; index < item.second.size(); ++index) {
      if (index > 0) {
        std::cerr << ",";
      }
      std::cerr << item.second[index];
    }
    std::cerr << "\n";
  }
  if (has_duplicates) {
    return SetError(error,
                    "下载任务中存在相同视频，"
                    "请去重后重新执行下载");
  }

  const std::filesystem::path output_directory = kOutputDirectory;
  // 单条重下覆盖旧文件；批量才启用唯一名，避免同名互相覆盖。
  const bool unique_names = tasks->size() > 1;
  std::set<std::filesystem::path> reserved_destinations;
  for (DownloadTask& task : *tasks) {
    task.destination =
        output_directory / SafeFileName(task.video, task.message_id);
    if (unique_names) {
      task.destination =
          MakeUniqueBatchDestination(task.destination, &reserved_destinations);
    }
  }
  return true;
}

void PrintFailures(const std::vector<Failure>& failures) {
  if (failures.empty()) {
    return;
  }

  std::cerr << "以下任务处理失败：\n";
  for (const Failure& failure : failures) {
    std::cerr << "  " << OneLine(failure.source) << "\n"
              << "    原因：" << OneLine(failure.reason) << "\n";
  }
}

class BatchDownloadRunner {
 public:
  BatchDownloadRunner(TelegramClient* client,
                      const std::vector<DownloadTask>& videos)
      : client_(client), videos_(videos) {}

  bool Run() {
    StartNext();
    while (!pending_by_file_id_.empty() || next_index_ < videos_.size()) {
      td::ClientManager::Response response = client_->Receive(10.0);
      if (response.object) {
        ProcessResponse(std::move(response));
      }
      FailStalledDownloads();
    }
    return failures_.empty();
  }

  const std::vector<Failure>& failures() const { return failures_; }

 private:
  void StartNext() {
    while (pending_by_file_id_.size() < kDownloadParallel &&
           next_index_ < videos_.size()) {
      const DownloadTask& task = videos_[next_index_];
      DownloadTask& pending = pending_by_file_id_[task.video.file_id];
      pending = task;
      pending.last_progress_time = std::chrono::steady_clock::now();
      const std::uint64_t request_id =
          client_->Send(td_api::make_object<td_api::downloadFile>(
              task.video.file_id, 32, 0, 0, false));
      file_id_by_request_id_[request_id] = task.video.file_id;
      ++next_index_;
    }
  }

  void FailDownload(std::int32_t file_id, const std::string& reason) {
    const auto pending = pending_by_file_id_.find(file_id);
    if (pending == pending_by_file_id_.end()) {
      return;
    }
    failures_.push_back(Failure{pending->second.source, reason});
    pending_by_file_id_.erase(pending);
    StartNext();
  }

  void FinishFileIfReady(const td_api::file& file) {
    const auto pending = pending_by_file_id_.find(file.id_);
    if (pending == pending_by_file_id_.end()) {
      return;
    }
    DownloadTask& active_download = pending->second;
    if (file.local_->downloaded_size_ > active_download.downloaded_size) {
      active_download.downloaded_size = file.local_->downloaded_size_;
      active_download.last_progress_time = std::chrono::steady_clock::now();
      ReportProgress(file, active_download);
    }
    if (!file.local_->is_downloading_completed_) {
      return;
    }

    SaveCompletedFile(file, active_download);

    pending_by_file_id_.erase(pending);
    StartNext();
  }

  void ReportProgress(const td_api::file& file, DownloadTask& download) {
    const std::int64_t total_size =
        file.size_ > 0 ? file.size_ : file.expected_size_;
    if (total_size <= 0) {
      return;
    }
    const std::int64_t downloaded_size =
        std::min(download.downloaded_size, total_size);
    int percent = static_cast<int>(downloaded_size * 100 / total_size);
    if (file.local_->is_downloading_completed_) {
      percent = 100;
    }
    if (percent < 100 && percent < download.last_reported_percent + 5) {
      return;
    }
    if (percent <= download.last_reported_percent) {
      return;
    }
    download.last_reported_percent = percent;
    std::cout << "下载进度："
              << OneLine(download.destination.filename().string()) << " "
              << percent << "% (" << FormatSize(downloaded_size) << "/"
              << FormatSize(total_size) << ")\n";
  }

  void SaveCompletedFile(const td_api::file& file, const DownloadTask& task) {
    std::string save_error;
    if (!SaveDownloadedFile(file.local_->path_, task.destination,
                            &save_error)) {
      failures_.push_back(Failure{task.source, save_error});
      return;
    }
    std::cout << "已下载：" << OneLine(task.destination.string()) << "\n";
    // 只为清理 TDLib 缓存，不关心结果。
    client_->Send(td_api::make_object<td_api::deleteFile>(file.id_));
  }

  void FailStalledDownloads() {
    const auto now = std::chrono::steady_clock::now();
    for (auto pending = pending_by_file_id_.begin();
         pending != pending_by_file_id_.end();) {
      const DownloadTask& active_download = pending->second;
      if (now - active_download.last_progress_time < kDownloadStallTimeout) {
        ++pending;
        continue;
      }

      const std::int32_t file_id = active_download.video.file_id;
      std::string reason = "下载超时（";
      reason += std::to_string(kDownloadStallTimeout.count());
      reason += " 分钟没有进度，已下载 ";
      reason += FormatSize(active_download.downloaded_size);
      reason += "）";
      client_->Send(td_api::make_object<td_api::cancelDownloadFile>(
          active_download.video.file_id, false));
      ++pending;
      FailDownload(file_id, reason);
    }
  }

  void ProcessResponse(td::ClientManager::Response response) {
    if (response.request_id != 0) {
      ProcessRequestResponse(std::move(response));
      return;
    }
    if (response.object->get_id() != td_api::updateFile::ID) {
      return;
    }

    auto update =
        td::move_tl_object_as<td_api::updateFile>(std::move(response.object));
    FinishFileIfReady(*update->file_);
  }

  void ProcessRequestResponse(td::ClientManager::Response response) {
    const auto request = file_id_by_request_id_.find(response.request_id);
    if (request == file_id_by_request_id_.end()) {
      return;
    }

    const std::int32_t file_id = request->second;
    if (response.object->get_id() == td_api::error::ID) {
      const auto& td_error =
          static_cast<const td_api::error&>(*response.object);
      std::string reason = "TDLib 错误 ";
      reason += std::to_string(td_error.code_);
      reason += "：";
      reason += OneLine(td_error.message_);
      FailDownload(file_id, reason);
    } else if (response.object->get_id() == td_api::file::ID) {
      const auto& file = static_cast<const td_api::file&>(*response.object);
      FinishFileIfReady(file);
    }
    file_id_by_request_id_.erase(request);
  }

  TelegramClient* client_ = nullptr;
  const std::vector<DownloadTask>& videos_;
  std::map<std::int32_t, DownloadTask> pending_by_file_id_;
  std::map<std::uint64_t, std::int32_t> file_id_by_request_id_;
  std::size_t next_index_ = 0;
  std::vector<Failure> failures_;
};

bool ParseDownloadRefs(const ParsedArgs& args, std::vector<MessageRef>* refs,
                       std::string* error) {
  const bool has_link = args.options.count("link") > 0;
  const bool has_links = args.options.count("links") > 0;
  const bool has_chat = args.options.count("chat") > 0;
  const bool has_message = args.options.count("message") > 0;

  if (has_link) {
    MessageRef ref;
    ref.is_link = true;
    ref.link = ParseStringOption(args, "link", "");
    refs->push_back(std::move(ref));
    return true;
  }

  if (has_links) {
    const std::string path = ParseStringOption(args, "links", "");
    return ReadLinksFromFile(path, refs, error);
  }

  if (!has_chat && !has_message) {
    return SetError(error,
                    "缺少下载参数：请使用 --chat/--message、"
                    "--link 或 --links");
  }

  MessageRef ref;
  if (!ParseInt64Option(args, "chat", &ref.chat_id, error)) {
    return false;
  }
  if (!ParseInt64Option(args, "message", &ref.message_id, error)) {
    return false;
  }
  refs->push_back(std::move(ref));
  return true;
}

}  // namespace

bool RunDownloadCommand(TelegramClient* client, const ParsedArgs& args,
                        std::string* error) {
  if (client == nullptr) {
    return SetError(error, "内部错误：Telegram client 为空");
  }

  std::vector<MessageRef> refs;
  if (!ParseDownloadRefs(args, &refs, error)) {
    return false;
  }

  std::vector<Failure> failures;
  std::vector<DownloadTask> tasks;
  tasks.reserve(refs.size());
  for (const MessageRef& ref : refs) {
    DownloadTask task;
    std::string reason;
    if (!ResolveRef(client, ref, &task, &reason)) {
      Failure failure;
      failure.source = RefSource(ref);
      failure.reason = reason;
      failures.push_back(std::move(failure));
      continue;
    }
    tasks.push_back(std::move(task));
  }

  std::string plan_error;
  if (!PlanTasks(&tasks, &plan_error)) {
    PrintFailures(failures);
    return SetError(error, plan_error);
  }
  if (tasks.empty()) {
    PrintFailures(failures);
    return SetError(error, "没有可下载的视频");
  }

  BatchDownloadRunner runner(client, tasks);
  runner.Run();
  failures.insert(failures.end(), runner.failures().begin(),
                  runner.failures().end());
  PrintFailures(failures);
  if (!failures.empty()) {
    return SetError(
        error, "有 " + std::to_string(failures.size()) + " 个任务处理失败");
  }
  return true;
}

}  // namespace tg_tools
