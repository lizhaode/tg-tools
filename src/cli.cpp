#include "cli.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

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
namespace {

std::filesystem::path PathDirectory(const std::filesystem::path& path) {
  std::error_code error;
  const std::filesystem::path canonical_path =
      std::filesystem::weakly_canonical(path, error);
  if (!error && !canonical_path.empty()) {
    return canonical_path.parent_path();
  }

  const std::filesystem::path absolute_path =
      std::filesystem::absolute(path, error);
  if (!error && !absolute_path.empty()) {
    return absolute_path.parent_path();
  }

  const std::filesystem::path current_path =
      std::filesystem::current_path(error);
  return error ? std::filesystem::path(".") : current_path;
}

std::filesystem::path ExecutableDirectory(const char* argv0) {
#if defined(__APPLE__)
  uint32_t path_size = 0;
  _NSGetExecutablePath(nullptr, &path_size);
  std::vector<char> executable_path(path_size);
  if (_NSGetExecutablePath(executable_path.data(), &path_size) == 0) {
    return PathDirectory(executable_path.data());
  }
#endif

  if (argv0 != nullptr && argv0[0] != '\0') {
    return PathDirectory(argv0);
  }
  return PathDirectory(".");
}

bool IsAllowedOption(const std::vector<std::string>& allowed_options,
                     const std::string& option) {
  for (const std::string& allowed_option : allowed_options) {
    if (option == allowed_option) {
      return true;
    }
  }
  return false;
}

bool ValidateCommandArgs(const std::string& command, const ParsedArgs& args,
                         std::string* error) {
  if (args.positional.size() != 1) {
    if (args.positional.empty()) {
      return SetError(error, "缺少命令");
    }
    return SetError(error, "未知参数：" + args.positional[1]);
  }

  std::vector<std::string> allowed_options;
  if (command == "chats") {
    allowed_options = {"limit"};
  } else if (command == "messages") {
    allowed_options = {"chat", "limit", "json"};
  } else if (command == "download") {
    allowed_options = {"chat", "message", "messages", "out"};
  } else if (command == "upload") {
    allowed_options = {"chat", "file", "caption", "json"};
  }

  for (const auto& option : args.options) {
    if (!IsAllowedOption(allowed_options, option.first)) {
      return SetError(error, "未知参数：--" + option.first);
    }
  }
  return true;
}

bool ValidateNoCommandArgs(const ParsedArgs& args, std::string* error) {
  if (args.options.empty() ||
      (args.options.size() == 1 && args.HasFlag("help"))) {
    return true;
  }
  return SetError(error, "未知参数：--" + args.options.begin()->first);
}

bool ValidateHelpArgs(const ParsedArgs& args, std::string* error) {
  if (args.positional.size() > 2) {
    return SetError(error, "未知参数：" + args.positional[2]);
  }
  if (!args.options.empty()) {
    return SetError(error, "未知参数：--" + args.options.begin()->first);
  }
  return true;
}

bool ValidateCommandHelpArgs(const ParsedArgs& args, std::string* error) {
  if (args.positional.size() != 1) {
    return SetError(error, "未知参数：" + args.positional[1]);
  }
  for (const auto& option : args.options) {
    if (option.first != "help") {
      return SetError(error, "未知参数：--" + option.first);
    }
  }
  return true;
}

}  // namespace

int RunCli(int argc, char** argv) {
  std::string error;
  const ParsedArgs args = ParseArgs(argc, argv);
  if (args.positional.empty()) {
    if (!ValidateNoCommandArgs(args, &error)) {
      std::cerr << "错误：" << OneLine(error) << '\n';
      return 1;
    }
    PrintHelp();
    return 0;
  }

  const std::string command = args.positional.front();

  if (command == "help") {
    if (!ValidateHelpArgs(args, &error)) {
      std::cerr << "错误：" << OneLine(error) << '\n';
      return 1;
    }
    const std::string help_command =
        args.positional.size() > 1 ? args.positional[1] : std::string();
    return PrintHelp(help_command) ? 0 : 1;
  }

  if (!IsKnownCommand(command)) {
    std::cerr << "未知命令：" << OneLine(command) << "\n\n";
    PrintHelp();
    return 1;
  }

  if (args.HasFlag("help")) {
    if (!ValidateCommandHelpArgs(args, &error)) {
      std::cerr << "错误：" << OneLine(error) << '\n';
      return 1;
    }
    return PrintHelp(command) ? 0 : 1;
  }

  if (!ValidateCommandArgs(command, args, &error)) {
    std::cerr << "错误：" << OneLine(error) << '\n';
    return 1;
  }

  Config config;
  const std::filesystem::path config_path =
      ExecutableDirectory(argc > 0 ? argv[0] : nullptr) / "telegram.conf";
  if (!LoadConfig(config_path, &config, &error)) {
    std::cerr << "错误：" << OneLine(error) << '\n';
    return 1;
  }
  TelegramClient client(std::move(config));
  if (!client.EnsureAuthorized(&error)) {
    std::cerr << "错误：" << OneLine(error) << '\n';
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
    std::cerr << "错误：" << OneLine(error) << '\n';
    return 1;
  }
  return 0;
}

}  // namespace tg_tools
