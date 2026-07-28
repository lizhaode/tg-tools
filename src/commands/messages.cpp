#include "commands/messages.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <string>
#include <utility>

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include "core/text_util.h"
#include "telegram/message_format.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

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
  const std::string json_path = ParseStringOption(args, "json", "");
  std::ofstream json_output;
  std::int64_t from_message_id = 0;
  int printed = 0;

  if (json_path.empty()) {
    PrintMessageHeader();
  } else {
    json_output.open(json_path);
    if (!json_output) {
      return SetError(error, "无法打开 JSON 输出文件：" + json_path);
    }
    json_output << "[\n";
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

    for (const auto& message : messages->messages_) {
      from_message_id = message->id_;
      if (json_output) {
        if (printed > 0) {
          json_output << ",\n";
        }
        WriteMessageJson(json_output, *message);
      } else {
        PrintMessageRow(*message);
      }
      ++printed;
      if (limit && printed >= *limit) {
        break;
      }
    }
  }

  if (json_output) {
    json_output << "\n]\n";
  }
  return true;
}

}  // namespace tg_tools
