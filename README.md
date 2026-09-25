# tg-tools

tg-tools 支持查看 Telegram 聊天消息，并下载消息里的视频。

## Quick Start

### 1. 环境要求

- 只支持 M 芯片 MacOS
- 先在 Telegram 官方申请好 `api_id` 和 `api_hash`

### 2. 安装依赖

```bash
xcode-select --install
brew install cmake gperf openssl@3
```

### 3. 编译

```bash
bash scripts/build.sh
```

可执行程序位于：

```bash
build/tg-tools
```

可以把它复制到任意目录使用：

```bash
mkdir -p ~/tg-tools
cp build/tg-tools ~/tg-tools/
```

### 4. 准备配置文件

参考 `config/telegram.example.conf` 编写自己的配置文件，并放到和二进制程序同级目录下。

### 5. 登录账号

```bash
./tg-tools login
```

### 6. 快速使用

列聊天：

```bash
./tg-tools chats --limit 50
```

看消息：

```bash
./tg-tools messages --chat <chat_id> --limit 50
```

下载视频：

```bash
./tg-tools download --chat <chat_id> --message <message_id>
./tg-tools download --link "https://t.me/用户名/消息ID"
./tg-tools download --links links.txt
```

## 功能详解

### login

```bash
./tg-tools login
```

根据配置文件指定路径创建 TDLib DB，并保存登录信息。

### chats

```bash
./tg-tools chats [--limit <数量>]
```

参数：

| 参数 | 说明 |
| --- | --- |
| `--limit <数量>` | 只读取最近的 N 个聊天；不指定时尝试加载全部聊天 |

输出格式：

| 字段 | 说明 |
| --- | --- |
| `chat_id` | 聊天 ID，后续命令都需要用它指定聊天或频道 |
| `type` | TDLib 原始类型名，例如 `chatTypePrivate`、`chatTypeBasicGroup`、`chatTypeSupergroup`、`chatTypeSecret`（channel 与 supergroup 同为 `chatTypeSupergroup`，不再区分） |
| `title` | 聊天标题 |

输出类似：

```text
chat_id            type               title
-1001234567890     chatTypeSupergroup My Channel
123456789          chatTypePrivate    Alice
```

示例：

```bash
./tg-tools chats --limit 50
```

### messages

```bash
./tg-tools messages --chat <chat_id> [--limit <数量>] [--csv <文件>]
```

参数：

| 参数 | 说明 |
| --- | --- |
| `--chat <chat_id>` | 必填；来自 `chats` 输出 |
| `--limit <数量>` | 最多读取 N 条消息；不指定时默认一直读取到没有更多消息 |
| `--csv <文件>` | 将消息写入 CSV 文件；不指定时命令行输出结果 |

命令行输出：

| 字段 | 说明 |
| --- | --- |
| `message_id` | 消息 ID，下载时通过它指定具体消息 |
| `date` | 消息时间 |
| `type` | TDLib 原始类型名，例如 `messageVideo`、`messageText` |
| `file` | 文件名，视频消息附带时长，如 `video.mp4 (42s)` |
| `quality` | 视频消息的清晰度与大小，如 `1080p 1.2G, 720p 480M`；第一个是原始版本，后面是服务器提供的备选清晰度；大小未知时显示 `-` |
| `text` | 消息文本或视频说明 |

命令行输出示例：

| message_id | date | type | file | quality | text |
| --- | --- | --- | --- | --- | --- |
| `12345` | `2026-07-28 12:00` | `messageVideo` | `video.mp4 (42s)` | `1080p 1.2G, 720p 480M, 480p 240M` | `caption` |
| `12346` | `2026-07-28 12:03` | `messageText` | | | `hello` |

示例：

```bash
./tg-tools messages --chat -1001234567890 --limit 50
./tg-tools messages --chat -1001234567890 --csv messages.csv
```

`messages --csv messages.csv` 输出示例（表头行）：

```text
message_id,date_text,type,file_id,file_name,mime_type,duration,width,height,size_text,supports_streaming,has_stickers,alternative_videos,text
12345,2026-07-28 12:00,messageVideo,654321,video.mp4,video/mp4,42,1920,1080,1.2G,true,false,"720p 480M h264",caption
```

### download

```bash
./tg-tools download --chat <chat_id> --message <message_id>
./tg-tools download --link <消息链接>
./tg-tools download --links <链接文件>
```

参数：

| 参数 | 说明 |
| --- | --- |
| `--chat <chat_id>` | 来自 `chats` 输出 |
| `--message <message_id>` | 来自 `messages` 输出 |
| `--link <消息链接>` | 一条消息链接 |
| `--links <链接文件>` | 每行一条链接 |

三种用法只能选一种，视频统一输出到
`downloads` 目录。

消息链接支持：

- 公开频道/群：`https://t.me/用户名/消息ID`
- 私有频道/群：`https://t.me/c/频道内码/消息ID`

链接文件里每行一条链接，空行和 `#` 开头的行
会被忽略。

下载时自动选择最清晰的版本：
比较原始文件与服务器提供的备选清晰度，
下载分辨率最高的一个。

输出规则：

- 文件名默认使用视频原始文件名
- 单条下载已存在会覆盖，
  批量下载会自动加 `-1`、`-2` 后缀
- 批量下载时单条失败不会中断，
  结束后统一列出失败链接
- 同一批里出现同一个视频会直接报错退出，
  不会下载

示例：

```bash
./tg-tools download --chat -1001234567890 --message 12345
./tg-tools download --link "https://t.me/example/123"
./tg-tools download --links links.txt
```

## 更多帮助

命令行内置帮助会列出当前可用命令和参数：

```bash
./tg-tools help
./tg-tools help messages
./tg-tools help download
```

也可以使用：

```bash
./tg-tools <命令> --help
```

## 开发

项目使用 Google C++ 风格配置，规则见 `.clang-format` 和 `.clang-tidy`。

需要本地运行格式化和静态检查时再安装 `llvm`：

```bash
brew install llvm
```
