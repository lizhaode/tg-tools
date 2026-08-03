#include "commands/messages.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include "core/text_util.h"
#include "telegram/message_format.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

bool WriteMessagesJson(
    const std::string& path,
    const std::vector<td_api::object_ptr<td_api::message>>& messages,
    std::string* error) {
  std::ofstream output(path);
  if (!output) {
    return SetError(error, "无法打开 JSON 输出文件：" + path);
  }

  output << "[\n";
  for (std::size_t index = 0; index < messages.size(); ++index) {
    if (index > 0) {
      output << ",\n";
    }
    WriteMessageJson(output, *messages[index]);
  }
  output << "\n]\n";
  output.close();
  if (!output) {
    return SetError(error, "写入 JSON 输出文件失败：" + path);
  }
  return true;
}

}  // namespace

bool RunMessagesCommand(TelegramClient* client, const ParsedArgs& args,
                        std::string* error) {
  if (client == nullptr) {
    return SetError(error, "内部错误：Telegram client 为空");
  }

  std::int64_t chat_id = 0;
  if (!ParseInt64Option(args, "chat", &chat_id, error)) {
    return false;
  }
  std::optional<int> limit;
  if (!ParseOptionalIntOption(args, "limit", &limit, error)) {
    return false;
  }
  if (limit && *limit <= 0) {
    return SetError(error, "参数 --limit 必须是正整数");
  }
  const std::string json_path = ParseStringOption(args, "json", "");
  const bool write_json = !json_path.empty();
  std::vector<td_api::object_ptr<td_api::message>> json_messages;
  std::int64_t from_message_id = 0;
  int printed = 0;

  if (!write_json) {
    PrintMessageHeader();
  }

  while (!limit || printed < *limit) {
    const int batch_limit = limit ? std::min(100, *limit - printed) : 100;
    auto result =
        client->Request(td_api::make_object<td_api::getChatHistory>(
                            chat_id, from_message_id, 0, batch_limit, false),
                        std::chrono::seconds(60), error);
    if (!result) {
      return false;
    }
    auto messages = td::move_tl_object_as<td_api::messages>(std::move(result));
    if (messages->messages_.empty()) {
      break;
    }

    for (auto& message : messages->messages_) {
      from_message_id = message->id_;
      if (write_json) {
        json_messages.push_back(std::move(message));
      } else {
        PrintMessageRow(*message);
      }
      ++printed;
      if (limit && printed >= *limit) {
        break;
      }
    }
  }

  if (write_json) {
    return WriteMessagesJson(json_path, json_messages, error);
  }
  return true;
}

}  // namespace tg_tools
