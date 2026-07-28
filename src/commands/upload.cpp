#include "commands/upload.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include "commands/upload_json.h"
#include "core/text_util.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

struct UploadFailure {
  std::filesystem::path name;
  std::string caption;
  std::string reason;
};

bool PathExists(const std::filesystem::path& path) {
  std::error_code error;
  return std::filesystem::exists(path, error) && !error;
}

bool UploadOneVideo(TelegramClient* client, std::int64_t chat_id,
                    const std::filesystem::path& file_path,
                    const std::string& caption, std::string* error) {
  if (client == nullptr) {
    return SetError(error, "内部错误：Telegram client 为空");
  }
  if (file_path.empty() || !PathExists(file_path)) {
    return SetError(error, "文件不存在：" + file_path.string());
  }

  auto local_file = td_api::make_object<td_api::inputFileLocal>();
  local_file->path_ = file_path.string();

  auto caption_text = td_api::make_object<td_api::formattedText>();
  caption_text->text_ = caption;

  auto input_video = td_api::make_object<td_api::inputVideo>();
  input_video->video_ = std::move(local_file);
  input_video->supports_streaming_ = true;

  auto video_content = td_api::make_object<td_api::inputMessageVideo>();
  video_content->video_ = std::move(input_video);
  video_content->caption_ = std::move(caption_text);

  auto send_message = td_api::make_object<td_api::sendMessage>();
  send_message->chat_id_ = chat_id;
  send_message->input_message_content_ = std::move(video_content);

  auto result =
      client->Request(std::move(send_message), std::chrono::minutes(30), error);
  if (!result) {
    return false;
  }
  auto message = td::move_tl_object_as<td_api::message>(std::move(result));
  std::cout << "已上传，message_id=" << message->id_ << '\n';
  return true;
}

void PrintUploadFailures(const std::vector<UploadFailure>& failures) {
  if (failures.empty()) {
    return;
  }

  std::cerr << "上传失败：\n";
  for (const UploadFailure& failure : failures) {
    std::cerr << "  name=" << failure.name.string()
              << " caption=" << failure.caption << " reason=" << failure.reason
              << '\n';
  }
}

}  // namespace

bool RunUploadCommand(TelegramClient* client, const ParsedArgs& args,
                      std::string* error) {
  std::int64_t chat_id = 0;
  if (!ParseInt64Option(args, "chat", &chat_id, error)) {
    return false;
  }
  const std::string json_path = ParseStringOption(args, "json", "");

  if (!json_path.empty()) {
    std::vector<UploadFailure> failures;
    std::vector<UploadItem> items;
    if (!ParseUploadJsonFile(json_path, &items, error)) {
      return false;
    }
    for (const UploadItem& item : items) {
      std::string item_error;
      if (!UploadOneVideo(client, chat_id, item.name, item.caption,
                          &item_error)) {
        failures.push_back(UploadFailure{item.name, item.caption, item_error});
      }
    }
    PrintUploadFailures(failures);
    if (!failures.empty()) {
      return SetError(error,
                      std::to_string(failures.size()) + " upload(s) failed");
    }
    return true;
  }

  const std::filesystem::path file_path = ParseStringOption(args, "file", "");
  const std::string caption = ParseStringOption(args, "caption", "");
  return UploadOneVideo(client, chat_id, file_path, caption, error);
}

}  // namespace tg_tools
