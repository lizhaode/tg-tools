#include "commands/chats.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
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

struct ChatRow {
  std::string chat_id;
  std::string type;
  std::string title;
};

void PrintChatRows(const std::vector<ChatRow>& rows) {
  std::size_t chat_id_width = DisplayWidth("chat_id");
  std::size_t type_width = DisplayWidth("type");
  for (const ChatRow& row : rows) {
    const std::size_t row_chat_id_width = DisplayWidth(row.chat_id);
    const std::size_t row_type_width = DisplayWidth(row.type);
    if (row_chat_id_width > chat_id_width) {
      chat_id_width = row_chat_id_width;
    }
    if (row_type_width > type_width) {
      type_width = row_type_width;
    }
  }
  chat_id_width += 2;
  type_width += 2;

  std::cout << PadRight("chat_id", chat_id_width)
            << PadRight("type", type_width) << "title\n";
  for (const ChatRow& row : rows) {
    std::cout << PadRight(row.chat_id, chat_id_width)
              << PadRight(row.type, type_width) << row.title << '\n';
  }
}

bool PrintChats(TelegramClient* client, int limit, std::string* error) {
  if (client == nullptr) {
    return SetError(error, "内部错误：Telegram client 为空");
  }

  auto result =
      client->Request(td_api::make_object<td_api::getChats>(nullptr, limit),
                      std::chrono::seconds(60), error);
  if (!result) {
    return false;
  }
  auto chats = td::move_tl_object_as<td_api::chats>(std::move(result));

  std::vector<ChatRow> rows;
  rows.reserve(chats->chat_ids_.size());
  for (const auto chat_id : chats->chat_ids_) {
    auto chat_object =
        client->Request(td_api::make_object<td_api::getChat>(chat_id),
                        std::chrono::seconds(30), error);
    if (!chat_object) {
      return false;
    }
    auto chat = td::move_tl_object_as<td_api::chat>(std::move(chat_object));
    rows.push_back(ChatRow{std::to_string(chat->id_), ChatTypeLabel(*chat),
                           OneLine(chat->title_)});
  }
  PrintChatRows(rows);
  return true;
}

bool LoadAllChats(TelegramClient* client, std::string* error) {
  if (client == nullptr) {
    return SetError(error, "内部错误：Telegram client 为空");
  }

  while (true) {
    auto result =
        client->Request(td_api::make_object<td_api::loadChats>(nullptr, 100),
                        std::chrono::seconds(60), error, true);
    if (!result) {
      return false;
    }
    if (result->get_id() == td_api::ok::ID) {
      continue;
    }
    if (result->get_id() == td_api::error::ID) {
      const auto& td_error = static_cast<const td_api::error&>(*result);
      if (td_error.code_ == 404) {
        return true;
      }
      return SetError(error, "TDLib error " + std::to_string(td_error.code_) +
                                 ": " + td_error.message_);
    }
  }
}

}  // namespace

bool RunChatsCommand(TelegramClient* client, const ParsedArgs& args,
                     std::string* error) {
  std::optional<int> limit;
  if (!ParseOptionalIntOption(args, "limit", &limit, error)) {
    return false;
  }
  if (limit) {
    if (*limit <= 0) {
      return SetError(error, "参数 --limit 必须是正整数");
    }
    return PrintChats(client, *limit, error);
  }

  if (!LoadAllChats(client, error)) {
    return false;
  }
  return PrintChats(client, std::numeric_limits<std::int32_t>::max(), error);
}

}  // namespace tg_tools
