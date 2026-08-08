#include "commands/download.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
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

constexpr int kDownloadParallel = 3;
constexpr std::chrono::minutes kDownloadStallTimeout(10);
constexpr std::chrono::seconds kDeleteFileTimeout(30);

struct PendingDownload {
  std::int64_t message_id = 0;
  std::int32_t file_id = 0;
  std::filesystem::path destination;
};

struct ActiveDownload {
  PendingDownload pending;
  std::int64_t downloaded_size = -1;
  int last_reported_percent = 0;
  std::chrono::steady_clock::time_point last_progress_time;
};

struct DownloadFailure {
  std::int64_t message_id = 0;
  std::int32_t file_id = 0;
  std::filesystem::path destination;
  std::string reason;
};

struct DownloadTask {
  VideoFile video;
  PendingDownload pending;
};

struct PendingDelete {
  PendingDownload pending;
  std::chrono::steady_clock::time_point started_at;
};

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

std::filesystem::path DownloadDestination(const std::filesystem::path& out,
                                          const VideoFile& video,
                                          std::int64_t message_id) {
  if (out.has_extension()) {
    return out;
  }
  return out / SafeFileName(video, message_id);
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

std::vector<std::string> SplitCsv(const std::string& value) {
  std::vector<std::string> items;
  std::stringstream stream(value);
  std::string item;
  while (std::getline(stream, item, ',')) {
    item = Trim(item);
    if (!item.empty()) {
      items.push_back(item);
    }
  }
  return items;
}

bool ParseMessageIds(const std::string& value,
                     std::vector<std::int64_t>* message_ids,
                     std::string* error) {
  if (message_ids == nullptr) {
    return SetError(error, "内部错误：消息 ID 输出指针为空");
  }
  message_ids->clear();
  for (const std::string& item : SplitCsv(value)) {
    std::int64_t message_id = 0;
    if (!ParseInt64Text(item, &message_id)) {
      return SetError(error, "消息 ID 必须是整数：" + item);
    }
    message_ids->push_back(message_id);
  }
  if (message_ids->empty()) {
    return SetError(error, "--messages 至少需要一个消息 ID");
  }
  return true;
}

void PrintDownloadFailures(const std::vector<DownloadFailure>& failures) {
  if (failures.empty()) {
    return;
  }

  std::cerr << "下载失败：\n";
  for (const DownloadFailure& failure : failures) {
    std::cerr << "  message_id=" << failure.message_id
              << " file_id=" << failure.file_id
              << " destination=" << OneLine(failure.destination.string())
              << " reason=" << OneLine(failure.reason) << '\n';
  }
}

bool ReportDuplicateFileIds(const std::vector<DownloadTask>& videos,
                            std::string* error) {
  std::map<std::int32_t, std::vector<std::int64_t>> message_ids_by_file_id;
  for (const DownloadTask& task : videos) {
    message_ids_by_file_id[task.video.file_id].push_back(
        task.pending.message_id);
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
        std::cerr << ',';
      }
      std::cerr << item.second[index];
    }
    std::cerr << '\n';
  }
  if (has_duplicates) {
    return SetError(error, "下载任务中存在相同视频，请去重后重新执行下载");
  }
  return true;
}

class BatchDownloadRunner {
 public:
  BatchDownloadRunner(TelegramClient* client,
                      const std::vector<DownloadTask>& videos)
      : client_(client), videos_(videos) {}

  bool Run(std::string* error) {
    StartNext();
    while (!pending_by_file_id_.empty() ||
           !pending_deletes_by_request_id_.empty() ||
           next_index_ < videos_.size()) {
      td::ClientManager::Response response = client_->Receive(10.0);
      if (response.object) {
        ProcessResponse(std::move(response));
      }
      FailStalledDownloads();
      FailStalledDeletes();
    }

    PrintDownloadFailures(failures_);
    if (!failures_.empty()) {
      return SetError(error,
                      std::to_string(failures_.size()) + " download(s) failed");
    }
    return true;
  }

 private:
  void StartNext() {
    while (active_ < kDownloadParallel && next_index_ < videos_.size()) {
      const DownloadTask& task = videos_[next_index_];
      pending_by_file_id_[task.video.file_id] =
          ActiveDownload{task.pending, -1, 0, std::chrono::steady_clock::now()};
      const std::uint64_t request_id =
          client_->Send(td_api::make_object<td_api::downloadFile>(
              task.video.file_id, 32, 0, 0, false));
      file_id_by_request_id_[request_id] = task.video.file_id;
      ++active_;
      ++next_index_;
    }
  }

  void FailDownload(std::int32_t file_id, const std::string& reason) {
    const auto pending = pending_by_file_id_.find(file_id);
    if (pending == pending_by_file_id_.end()) {
      return;
    }
    failures_.push_back(DownloadFailure{
        pending->second.pending.message_id, pending->second.pending.file_id,
        pending->second.pending.destination, reason});
    pending_by_file_id_.erase(pending);
    --active_;
    StartNext();
  }

  void FinishFileIfReady(const td_api::file& file) {
    const auto pending = pending_by_file_id_.find(file.id_);
    if (pending == pending_by_file_id_.end()) {
      return;
    }
    ActiveDownload& active_download = pending->second;
    if (file.local_->downloaded_size_ > active_download.downloaded_size) {
      active_download.downloaded_size = file.local_->downloaded_size_;
      active_download.last_progress_time = std::chrono::steady_clock::now();
      ReportProgress(file, active_download);
    }
    if (!file.local_->is_downloading_completed_) {
      return;
    }

    if (file.local_->path_.empty() || !PathExists(file.local_->path_)) {
      failures_.push_back(DownloadFailure{
          active_download.pending.message_id, file.id_,
          active_download.pending.destination,
          "TDLib reported completion but local file is missing"});
    } else {
      SaveCompletedFile(file, active_download.pending);
    }

    pending_by_file_id_.erase(pending);
    --active_;
    StartNext();
  }

  void ReportProgress(const td_api::file& file, ActiveDownload& download) {
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
              << OneLine(download.pending.destination.filename().string())
              << ' ' << percent << "% (" << FormatSize(downloaded_size) << "/"
              << FormatSize(total_size) << ")\n";
  }

  void SaveCompletedFile(const td_api::file& file,
                         const PendingDownload& pending) {
    std::string save_error;
    if (!SaveDownloadedFile(file.local_->path_, pending.destination,
                            &save_error)) {
      failures_.push_back(DownloadFailure{pending.message_id, file.id_,
                                          pending.destination, save_error});
      return;
    }
    std::cout << "已下载：" << OneLine(pending.destination.string()) << '\n';
    const std::uint64_t request_id =
        client_->Send(td_api::make_object<td_api::deleteFile>(file.id_));
    pending_deletes_by_request_id_[request_id] =
        PendingDelete{pending, std::chrono::steady_clock::now()};
  }

  void FailStalledDownloads() {
    const auto now = std::chrono::steady_clock::now();
    for (auto pending = pending_by_file_id_.begin();
         pending != pending_by_file_id_.end();) {
      const ActiveDownload& active_download = pending->second;
      if (now - active_download.last_progress_time < kDownloadStallTimeout) {
        ++pending;
        continue;
      }

      const std::int32_t file_id = active_download.pending.file_id;
      const std::string reason =
          "download stalled for " +
          std::to_string(kDownloadStallTimeout.count()) +
          " minutes, downloaded_size=" +
          std::to_string(active_download.downloaded_size);
      client_->Send(td_api::make_object<td_api::cancelDownloadFile>(
          active_download.pending.file_id, false));
      ++pending;
      FailDownload(file_id, reason);
    }
  }

  void FailStalledDeletes() {
    const auto now = std::chrono::steady_clock::now();
    for (auto pending = pending_deletes_by_request_id_.begin();
         pending != pending_deletes_by_request_id_.end();) {
      if (now - pending->second.started_at < kDeleteFileTimeout) {
        ++pending;
        continue;
      }
      std::cerr << "警告：已下载 "
                << OneLine(pending->second.pending.destination.string())
                << "，但清理 TDLib 缓存超时\n";
      pending = pending_deletes_by_request_id_.erase(pending);
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
    const auto pending_delete =
        pending_deletes_by_request_id_.find(response.request_id);
    if (pending_delete != pending_deletes_by_request_id_.end()) {
      ProcessDeleteResponse(std::move(response), pending_delete);
      return;
    }

    const auto request = file_id_by_request_id_.find(response.request_id);
    if (request == file_id_by_request_id_.end()) {
      return;
    }

    const std::int32_t file_id = request->second;
    if (response.object->get_id() == td_api::error::ID) {
      const auto& td_error =
          static_cast<const td_api::error&>(*response.object);
      FailDownload(file_id, "TDLib error " + std::to_string(td_error.code_) +
                                ": " + td_error.message_);
    } else if (response.object->get_id() == td_api::file::ID) {
      const auto& file = static_cast<const td_api::file&>(*response.object);
      FinishFileIfReady(file);
    }
    file_id_by_request_id_.erase(request);
  }

  void ProcessDeleteResponse(
      td::ClientManager::Response response,
      std::map<std::uint64_t, PendingDelete>::iterator pending_delete) {
    const PendingDownload pending = pending_delete->second.pending;
    pending_deletes_by_request_id_.erase(pending_delete);

    if (response.object->get_id() == td_api::ok::ID) {
      return;
    }
    if (response.object->get_id() == td_api::error::ID) {
      const auto& td_error =
          static_cast<const td_api::error&>(*response.object);
      std::cerr << "警告：已下载 " << OneLine(pending.destination.string())
                << "，但清理 TDLib 缓存失败：" << td_error.message_ << '\n';
      return;
    }
    std::cerr << "警告：已下载 " << OneLine(pending.destination.string())
              << "，但 TDLib 返回了意外的删除响应\n";
  }

  TelegramClient* client_ = nullptr;
  const std::vector<DownloadTask>& videos_;
  std::map<std::int32_t, ActiveDownload> pending_by_file_id_;
  std::map<std::uint64_t, std::int32_t> file_id_by_request_id_;
  std::map<std::uint64_t, PendingDelete> pending_deletes_by_request_id_;
  std::size_t next_index_ = 0;
  int active_ = 0;
  std::vector<DownloadFailure> failures_;
};

bool DownloadVideosParallel(TelegramClient* client,
                            const std::vector<DownloadTask>& videos,
                            std::string* error) {
  BatchDownloadRunner runner(client, videos);
  return runner.Run(error);
}

}  // namespace

bool RunDownloadCommand(TelegramClient* client, const ParsedArgs& args,
                        std::string* error) {
  if (client == nullptr) {
    return SetError(error, "内部错误：Telegram client 为空");
  }

  std::int64_t chat_id = 0;
  if (!ParseInt64Option(args, "chat", &chat_id, error)) {
    return false;
  }
  const std::filesystem::path out = ParseStringOption(args, "out", "downloads");
  const std::string messages = ParseStringOption(args, "messages", "");
  const bool has_message = args.options.find("message") != args.options.end();
  const bool has_messages = args.options.find("messages") != args.options.end();

  if (has_message && has_messages) {
    return SetError(error, "--message 和 --messages 只能二选一");
  }

  if (!messages.empty()) {
    if (out.has_extension()) {
      return SetError(error, "使用 --messages 时，--out 必须是目录");
    }
    std::vector<DownloadTask> videos;
    std::vector<DownloadFailure> failures;
    std::vector<std::int64_t> message_ids;
    std::set<std::filesystem::path> reserved_destinations;
    if (!ParseMessageIds(messages, &message_ids, error)) {
      return false;
    }
    for (const std::int64_t message_id : message_ids) {
      std::string item_error;
      auto message_object = client->Request(
          td_api::make_object<td_api::getMessage>(chat_id, message_id),
          std::chrono::seconds(30), &item_error);
      if (!message_object) {
        failures.push_back(DownloadFailure{message_id, 0, {}, item_error});
        continue;
      }
      auto message =
          td::move_tl_object_as<td_api::message>(std::move(message_object));
      std::optional<VideoFile> video = ExtractVideoFile(*message);
      if (!video) {
        failures.push_back(
            DownloadFailure{message_id,
                            0,
                            {},
                            "message does not contain a downloadable video"});
        continue;
      }
      const std::filesystem::path destination = MakeUniqueBatchDestination(
          out / SafeFileName(*video, message_id), &reserved_destinations);
      videos.push_back(DownloadTask{
          *video, PendingDownload{message_id, video->file_id, destination}});
    }
    PrintDownloadFailures(failures);
    if (!ReportDuplicateFileIds(videos, error)) {
      return false;
    }
    if (!DownloadVideosParallel(client, videos, error)) {
      return false;
    }
    if (!failures.empty()) {
      return SetError(error,
                      std::to_string(failures.size()) +
                          " message(s) could not be prepared for download");
    }
    return true;
  }

  std::int64_t message_id = 0;
  if (!ParseInt64Option(args, "message", &message_id, error)) {
    return false;
  }
  auto message_object = client->Request(
      td_api::make_object<td_api::getMessage>(chat_id, message_id),
      std::chrono::seconds(30), error);
  if (!message_object) {
    return false;
  }
  auto message =
      td::move_tl_object_as<td_api::message>(std::move(message_object));
  std::optional<VideoFile> video = ExtractVideoFile(*message);
  if (!video) {
    return SetError(error,
                    "selected message does not contain a downloadable video");
  }
  std::vector<DownloadTask> videos;
  videos.push_back(DownloadTask{
      *video, PendingDownload{message_id, video->file_id,
                              DownloadDestination(out, *video, message_id)}});
  return DownloadVideosParallel(client, videos, error);
}

}  // namespace tg_tools
