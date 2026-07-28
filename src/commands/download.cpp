#include "commands/download.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
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

struct PendingDownload {
  std::int64_t message_id = 0;
  std::filesystem::path destination;
};

struct DownloadFailure {
  std::int64_t message_id = 0;
  std::int32_t file_id = 0;
  std::filesystem::path destination;
  std::string reason;
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

bool DownloadOneVideo(TelegramClient* client, const VideoFile& video,
                      const std::filesystem::path& destination,
                      std::string* error) {
  auto file_object = client->Request(
      td_api::make_object<td_api::downloadFile>(video.file_id, 32, 0, 0, true),
      std::chrono::minutes(30), error);
  if (!file_object) {
    return false;
  }
  auto file = td::move_tl_object_as<td_api::file>(std::move(file_object));
  const std::filesystem::path local_path = file->local_->path_;
  if (local_path.empty() || !PathExists(local_path)) {
    return SetError(
        error,
        "TDLib reported download complete but local file path is empty or "
        "missing");
  }

  if (!SaveDownloadedFile(local_path, destination, error)) {
    return false;
  }
  auto delete_result =
      client->Request(td_api::make_object<td_api::deleteFile>(video.file_id),
                      std::chrono::seconds(30), error);
  if (!delete_result) {
    return false;
  }
  std::cout << "已下载：" << destination.string() << '\n';
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
              << " destination=" << failure.destination.string()
              << " reason=" << failure.reason << '\n';
  }
}

bool DownloadVideosParallel(
    TelegramClient* client,
    const std::vector<std::pair<VideoFile, PendingDownload>>& videos,
    std::string* error) {
  std::map<std::int32_t, PendingDownload> pending_by_file_id;
  std::size_t next_index = 0;
  int active = 0;
  std::vector<DownloadFailure> failures;

  auto start_next = [&]() {
    while (active < kDownloadParallel && next_index < videos.size()) {
      const VideoFile& video = videos[next_index].first;
      const PendingDownload& pending = videos[next_index].second;
      pending_by_file_id[video.file_id] = pending;
      client->Send(td_api::make_object<td_api::downloadFile>(video.file_id, 32,
                                                             0, 0, false));
      ++active;
      ++next_index;
    }
  };

  start_next();
  while (!pending_by_file_id.empty() || next_index < videos.size()) {
    auto response = client->Receive(10.0);
    if (!response.object) {
      continue;
    }
    if (response.object->get_id() != td_api::updateFile::ID) {
      continue;
    }

    auto update =
        td::move_tl_object_as<td_api::updateFile>(std::move(response.object));
    const td_api::file& file = *update->file_;
    const auto pending = pending_by_file_id.find(file.id_);
    if (pending == pending_by_file_id.end()) {
      continue;
    }
    if (!file.local_->is_downloading_completed_) {
      continue;
    }
    if (file.local_->path_.empty() || !PathExists(file.local_->path_)) {
      failures.push_back(DownloadFailure{
          pending->second.message_id, file.id_, pending->second.destination,
          "TDLib reported completion but local file is missing"});
      pending_by_file_id.erase(pending);
      --active;
      start_next();
      continue;
    }

    std::string save_error;
    if (SaveDownloadedFile(file.local_->path_, pending->second.destination,
                           &save_error)) {
      client->Send(td_api::make_object<td_api::deleteFile>(file.id_));
      std::cout << "已下载：" << pending->second.destination.string() << '\n';
    } else {
      failures.push_back(DownloadFailure{pending->second.message_id, file.id_,
                                         pending->second.destination,
                                         save_error});
    }
    pending_by_file_id.erase(pending);
    --active;
    start_next();
  }

  PrintDownloadFailures(failures);
  if (!failures.empty()) {
    return SetError(error,
                    std::to_string(failures.size()) + " download(s) failed");
  }
  return true;
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

  if (!messages.empty()) {
    if (out.has_extension()) {
      return SetError(error, "使用 --messages 时，--out 必须是目录");
    }
    std::vector<std::pair<VideoFile, PendingDownload>> videos;
    std::vector<DownloadFailure> failures;
    std::vector<std::int64_t> message_ids;
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
      videos.push_back(
          {*video, PendingDownload{message_id,
                                   out / SafeFileName(*video, message_id)}});
    }
    PrintDownloadFailures(failures);
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
  return DownloadOneVideo(client, *video,
                          DownloadDestination(out, *video, message_id), error);
}

}  // namespace tg_tools
