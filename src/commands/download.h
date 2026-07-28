#ifndef TG_TOOLS_SRC_COMMANDS_DOWNLOAD_H_
#define TG_TOOLS_SRC_COMMANDS_DOWNLOAD_H_

#include <string>

#include "core/args.h"
#include "telegram/telegram_client.h"

namespace tg_tools {

bool RunDownloadCommand(TelegramClient* client, const ParsedArgs& args,
                        std::string* error);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_COMMANDS_DOWNLOAD_H_
