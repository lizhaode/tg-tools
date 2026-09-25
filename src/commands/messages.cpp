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

bool WriteMessagesCsv(const std::string& path,
                      const std::vector<MessageRow>& rows, std::string* error) {
  std::ofstream output(path);
  if (!output) {
    return SetError(error, "无法打开 CSV 输出文件：" + path);
  }

  output << "message_id,date_text,type,file_id,file_name,mime_type,duration,"
            "width,height,size_text,supports_streaming,has_stickers,"
            "alternative_videos,text\n";
  for (const MessageRow& row : rows) {
    WriteMessageCsv(output, row);
  }
  output.close();
  if (!output) {
    return SetError(error, "写入 CSV 输出文件失败：" + path);
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
  const bool has_csv = args.options.find("csv") != args.options.end();
  const std::string csv_path = ParseStringOption(args, "csv", "");
  if (has_csv && (csv_path.empty() || csv_path == "true")) {
    return SetError(error, "参数 --csv 需要一个文件路径");
  }
  const bool write_csv = has_csv;
  std::vector<td_api::object_ptr<td_api::message>> collected;
  std::int64_t from_message_id = 0;
  int printed = 0;

  if (!client->LoadAllChats(error)) {
    return false;
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
    auto batch = td::move_tl_object_as<td_api::messages>(std::move(result));
    if (batch->messages_.empty()) {
      break;
    }

    for (auto& message : batch->messages_) {
      from_message_id = message->id_;
      collected.push_back(std::move(message));
      ++printed;
      if (limit && printed >= *limit) {
        break;
      }
    }
  }

  const std::vector<MessageRow> rows = DescribeMessages(collected);
  if (write_csv) {
    return WriteMessagesCsv(csv_path, rows, error);
  }
  PrintMessageHeader();
  for (const MessageRow& row : rows) {
    PrintMessageRow(row);
  }
  return true;
}

}  // namespace tg_tools
