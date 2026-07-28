#include "help.h"

#include <iostream>

namespace tg_tools {
namespace {

void PrintGeneralHelp() {
  std::cout << R"(tg-tools - Telegram 视频命令行工具

用途：登录 Telegram，查看聊天和消息，下载消息里的视频，上传本地视频。

用法：
  ./build/tg-tools <命令> [选项]
  ./build/tg-tools help [命令]
  ./build/tg-tools <命令> --help

准备：
  cp config/telegram.example.conf config/telegram.conf
  编辑 config/telegram.conf，填写 api_id 和 api_hash。
  如需代理，先执行：export ALL_PROXY=http://127.0.0.1:7890

命令：
  login      登录账号，首次运行会要求手机号、验证码、两步验证密码
  chats      列出聊天，输出 chat_id、类型、标题
  messages   读取聊天消息，默认表格输出，也可导出 JSON
  download   下载单条或多条消息中的视频文件
  upload     上传单个视频，或按 JSON 列表批量上传

常用示例：
  ./build/tg-tools login
  ./build/tg-tools chats --limit 50
  ./build/tg-tools messages --chat -1001234567890 --limit 20
  ./build/tg-tools messages --chat -1001234567890 --json messages.json
  ./build/tg-tools download --chat -1001234567890 --message 12345 --out downloads
  ./build/tg-tools upload --chat -1009876543210 --file downloads/a.mp4 --caption "已保存"

更多帮助：
  ./build/tg-tools help messages
  ./build/tg-tools download --help
)";
}

bool PrintCommandHelp(const std::string& command) {
  if (command == "login") {
    std::cout << R"(login - 登录 Telegram 账号

用法：
  ./build/tg-tools login

说明：
  首次运行会按提示输入手机号、验证码，必要时输入两步验证密码。
  登录状态保存在 config/telegram.conf 指定的 TDLib 数据目录中。
  已登录后再次运行会直接显示授权成功。
)";
    return true;
  }

  if (command == "chats") {
    std::cout << R"(chats - 列出聊天

用法：
  ./build/tg-tools chats [--limit 数量]

选项：
  --limit 数量   只读取最近的 N 个聊天；不指定时尝试加载全部聊天

输出：
  chat_id        聊天 ID，后续 messages/download/upload 要用
  type           private、group、supergroup、channel 等类型
  title          聊天标题

示例：
  ./build/tg-tools chats --limit 50
)";
    return true;
  }

  if (command == "messages") {
    std::cout << R"(messages - 读取聊天消息

用法：
  ./build/tg-tools messages --chat 聊天ID [--limit 数量] [--json 文件]

选项：
  --chat 聊天ID   必填；来自 chats 输出的 chat_id
  --limit 数量    最多读取 N 条消息；默认一直读取到没有更多消息
  --json 文件     写入 JSON 数组；不指定时输出表格

输出：
  表格列为 message_id、date、type、file、text。
  JSON 包含文件 ID、文件名、MIME 类型、尺寸、时长和文本等字段。

示例：
  ./build/tg-tools messages --chat -1001234567890 --limit 20
  ./build/tg-tools messages --chat -1001234567890 --json messages.json
)";
    return true;
  }

  if (command == "download") {
    std::cout << R"(download - 下载消息中的视频

用法：
  ./build/tg-tools download --chat 聊天ID --message 消息ID [--out 路径]
  ./build/tg-tools download --chat 聊天ID --messages ID1,ID2,ID3 --out 目录

选项：
  --chat 聊天ID       必填；来自 chats 输出
  --message 消息ID    下载单条消息中的视频
  --messages ID列表   批量下载，多条 ID 用英文逗号分隔
  --out 路径          输出目录或文件路径；默认 downloads

说明：
  单条下载时，--out 有扩展名则当作文件路径，否则当作目录。
  批量下载时，--out 必须是目录。失败项会打印 message_id、file_id 和原因。

示例：
  ./build/tg-tools download --chat -1001234567890 --message 12345 --out downloads
  ./build/tg-tools download --chat -1001234567890 --messages 12345,12346 --out downloads
)";
    return true;
  }

  if (command == "upload") {
    std::cout << R"(upload - 上传视频

用法：
  ./build/tg-tools upload --chat 聊天ID --file 文件 [--caption 文本]
  ./build/tg-tools upload --chat 聊天ID --json 文件

选项：
  --chat 聊天ID     必填；目标聊天 ID
  --file 文件       上传单个本地视频文件
  --caption 文本    单个文件的说明文字
  --json 文件       批量上传列表，格式为 [{"name":"a.mp4","caption":"说明"}]

示例：
  ./build/tg-tools upload --chat -1009876543210 --file downloads/a.mp4 --caption "已保存"
  ./build/tg-tools upload --chat -1009876543210 --json uploads.json
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
  std::cerr << "未知命令：" << command << "\n\n";
  PrintGeneralHelp();
  return false;
}

}  // namespace tg_tools
