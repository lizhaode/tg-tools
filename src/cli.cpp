#include "cli.h"

#include <iostream>
#include <string>
#include <utility>

#include "commands/chats.h"
#include "commands/download.h"
#include "commands/messages.h"
#include "commands/upload.h"
#include "core/args.h"
#include "core/config.h"
#include "core/text_util.h"
#include "help.h"
#include "telegram/telegram_client.h"

namespace tg_tools {

int RunCli(int argc, char** argv) {
  std::string error;
  const ParsedArgs args = ParseArgs(argc, argv);
  if (args.positional.empty()) {
    PrintHelp();
    return 0;
  }

  const std::string command = args.positional.front();

  if (command == "help") {
    const std::string help_command =
        args.positional.size() > 1 ? args.positional[1] : std::string();
    return PrintHelp(help_command) ? 0 : 1;
  }

  if (args.HasFlag("help")) {
    return PrintHelp(command) ? 0 : 1;
  }

  if (!IsKnownCommand(command)) {
    std::cerr << "未知命令：" << command << "\n\n";
    PrintHelp();
    return 1;
  }

  Config config;
  if (!LoadConfig("config/telegram.conf", &config, &error)) {
    std::cerr << "错误：" << error << '\n';
    return 1;
  }
  TelegramClient client(std::move(config));
  if (!client.EnsureAuthorized(&error)) {
    std::cerr << "错误：" << error << '\n';
    return 1;
  }

  bool ok = true;
  if (command == "login") {
    std::cout << "登录已完成，授权状态正常。\n";
  } else if (command == "chats") {
    ok = RunChatsCommand(&client, args, &error);
  } else if (command == "messages") {
    ok = RunMessagesCommand(&client, args, &error);
  } else if (command == "download") {
    ok = RunDownloadCommand(&client, args, &error);
  } else if (command == "upload") {
    ok = RunUploadCommand(&client, args, &error);
  } else {
    ok = SetError(&error, "未知命令：" + command);
  }
  if (!ok) {
    std::cerr << "错误：" << error << '\n';
    return 1;
  }
  return 0;
}

}  // namespace tg_tools
