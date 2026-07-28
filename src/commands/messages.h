#ifndef TG_TOOLS_SRC_COMMANDS_MESSAGES_H_
#define TG_TOOLS_SRC_COMMANDS_MESSAGES_H_

#include <string>

#include "core/args.h"
#include "telegram/telegram_client.h"

namespace tg_tools {

bool RunMessagesCommand(TelegramClient* client, const ParsedArgs& args,
                        std::string* error);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_COMMANDS_MESSAGES_H_
