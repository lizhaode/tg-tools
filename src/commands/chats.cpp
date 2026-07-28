#include "commands/chats.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include "core/text_util.h"
#include "telegram/message_format.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

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

  std::cout << "chat_id\ttype\ttitle\n";
  for (const auto chat_id : chats->chat_ids_) {
    auto chat_object =
        client->Request(td_api::make_object<td_api::getChat>(chat_id),
                        std::chrono::seconds(30), error);
    if (!chat_object) {
      return false;
    }
    auto chat = td::move_tl_object_as<td_api::chat>(std::move(chat_object));
    std::cout << chat->id_ << '\t' << ChatTypeLabel(*chat) << '\t'
              << chat->title_ << '\n';
  }
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
    return PrintChats(client, *limit, error);
  }

  if (!LoadAllChats(client, error)) {
    return false;
  }
  return PrintChats(client, std::numeric_limits<std::int32_t>::max(), error);
}

}  // namespace tg_tools
