#include "help.h"

#include <iostream>

#include "core/text_util.h"

namespace tg_tools {
namespace {

void PrintGeneralHelp() {
  std::cout << R"(tg-tools - Telegram 视频命令行工具

功能：查看 Telegram 聊天消息、下载消息里的视频，支持批量下载。

用法：
  ./tg-tools <命令> [选项]
  ./tg-tools help [命令]
  ./tg-tools <命令> --help  （或 -h）

配置：
  参考示例配置编写 telegram.conf，并放到二进制程序同级目录。
  如需代理，先执行：export ALL_PROXY=http://127.0.0.1:7890

命令：
  login      登录 Telegram 账号
  chats      列出聊天，输出 chat_id、类型、标题
  messages   读取聊天消息，默认表格输出，也可导出 CSV
  download   下载消息中的视频，支持消息链接和链接文件

每个命令都有独立帮助：
  ./tg-tools help <命令>
  ./tg-tools <命令> --help （或 -h）
)";
}

bool PrintCommandHelp(const std::string& command) {
  if (command == "login") {
    std::cout << R"(login - 登录 Telegram 账号

用法：
  ./tg-tools login
)";
    return true;
  }

  if (command == "chats") {
    std::cout << R"(chats - 列出聊天

功能：
  列出 Telegram 聊天，获取后续命令使用的 chat_id。

用法：
  ./tg-tools chats [--limit 数量]

参数：
  --limit 数量   只读取最近的 N 个聊天；不指定时加载全部聊天
)";
    return true;
  }

  if (command == "messages") {
    std::cout << R"(messages - 读取聊天消息

功能：
  读取指定聊天的消息，支持命令行表格输出或 CSV 文件输出。
  视频消息的表格 quality 列和 CSV 输出会展示原始及备选清晰度
  （分辨率、大小、编码），便于下载前查看。

用法：
  ./tg-tools messages --chat 聊天ID [--limit 数量] [--csv 文件]

参数：
  --chat 聊天ID   必填；来自 chats 输出的 chat_id
  --limit 数量    最多读取 N 条消息；不指定时一直读取到没有更多消息
  --csv 文件      写入 CSV 文件；不指定时命令行输出
)";
    return true;
  }

  if (command == "download") {
    std::cout << R"(download - 下载消息中的视频

功能：
  下载单条消息中的视频，或按链接批量下载。
  自动选择最清晰的版本（原始文件或服务器提供的
  备选清晰度中分辨率最高的一个）。
  文件输出到 downloads 目录。

用法：
  ./tg-tools download --chat 聊天ID --message 消息ID
  ./tg-tools download --link 消息链接
  ./tg-tools download --links 链接文件

参数：
  --chat 聊天ID     来自 chats 输出的 chat_id
  --message 消息ID  来自 messages 输出的 message_id
  --link 消息链接   例如 t.me/用户名/消息ID
  --links 链接文件  每行一个消息链接；
                    空行和 # 开头忽略

要求：
  以上三种用法只能选一种。
  下载时会显示进度（每 5% 更新一次）；
  最多同时下载 3 个视频。
  批量下载时单行失败不会中断，
  结束后统一列出失败的链接。
)";
    return true;
  }

  return false;
}

}  // namespace

bool IsKnownCommand(const std::string& command) {
  return command == "login" || command == "chats" || command == "messages" ||
         command == "download";
}

bool PrintHelp(const std::string& command) {
  if (command.empty()) {
    PrintGeneralHelp();
    return true;
  }
  if (PrintCommandHelp(command)) {
    return true;
  }
  std::cerr << "未知命令：" << OneLine(command) << "\n\n";
  PrintGeneralHelp();
  return false;
}

}  // namespace tg_tools
