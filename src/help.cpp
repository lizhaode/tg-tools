#include "help.h"

#include <iostream>

#include "core/text_util.h"

namespace tg_tools {
namespace {

void PrintGeneralHelp() {
  std::cout << R"(tg-tools - Telegram 视频命令行工具

功能：查看 Telegram 聊天消息、下载消息里的视频、上传本地视频，支持批量下载和批量上传。

用法：
  ./tg-tools <命令> [选项]
  ./tg-tools help [命令]
  ./tg-tools <命令> --help

配置：
  参考示例配置编写 telegram.conf，并放到二进制程序同级目录。
  如需代理，先执行：export ALL_PROXY=http://127.0.0.1:7890

命令：
  login      登录 Telegram 账号
  chats      列出聊天，输出 chat_id、类型、标题
  messages   读取聊天消息，默认表格输出，也可导出 JSON
  download   下载单条或多条消息中的视频文件
  upload     上传单个视频，或按 JSON 列表批量上传

每个命令都有独立帮助：
  ./tg-tools help <命令>
  ./tg-tools <命令> --help
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
  读取指定聊天的消息，支持命令行表格输出或 JSON 文件输出。

用法：
  ./tg-tools messages --chat 聊天ID [--limit 数量] [--json 文件]

参数：
  --chat 聊天ID   必填；来自 chats 输出的 chat_id
  --limit 数量    最多读取 N 条消息；不指定时一直读取到没有更多消息
  --json 文件     写入 JSON 数组；不指定时命令行输出
)";
    return true;
  }

  if (command == "download") {
    std::cout << R"(download - 下载消息中的视频

功能：
  下载单条或多条消息中的视频文件。

用法：
  ./tg-tools download --chat 聊天ID --message 消息ID [--out 路径]
  ./tg-tools download --chat 聊天ID --messages ID1,ID2,ID3 --out 目录

参数：
  --chat 聊天ID       必填；来自 chats 输出
  --message 消息ID    下载单条消息中的视频；与 --messages 二选一
  --messages ID列表   批量下载，多条 ID 用英文逗号分隔
  --out 路径          输出目录或文件路径；默认 downloads

要求：
  单条下载：使用 --message；--out 可以是目录或完整文件路径。
  批量下载：使用 --messages；--out 必须是目录。
  下载时会显示进度（每 5% 更新一次）；最多同时下载 3 个视频。
)";
    return true;
  }

  if (command == "upload") {
    std::cout << R"(upload - 上传视频

功能：
  上传单个本地视频，或按 JSON 列表批量上传视频。

用法：
  ./tg-tools upload --chat 聊天ID --file 文件 [--caption 文本]
  ./tg-tools upload --chat 聊天ID --json 文件

参数：
  --chat 聊天ID     必填；目标聊天 ID
  --file 文件       上传单个本地视频文件；与 --json 二选一
  --caption 文本    单个文件的说明文字
  --json 文件       批量上传列表，格式为 [{"name":"a.mp4","caption":"说明"}]

要求：
  --json 文件必须是数组；每个对象的 name 必填，caption 可选。
  批量上传最多同时处理 3 个视频；每个视频会显示进度并等待最终结果。
)";
    return true;
  }

  return false;
}

}  // namespace

bool IsKnownCommand(const std::string& command) {
  return command == "login" || command == "chats" || command == "messages" ||
         command == "download" || command == "upload";
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
