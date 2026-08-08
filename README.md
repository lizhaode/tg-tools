# tg-tools

tg-tools 支持查看 Telegram 聊天消息、下载消息里的视频，以及将本地视频单个或批量上传到 Telegram。

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
./tg-tools download --chat <chat_id> --message <message_id> --out downloads
```

上传视频：

```bash
./tg-tools upload --chat <chat_id> --file downloads/video.mp4 --caption "saved locally"
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
./tg-tools messages --chat <chat_id> [--limit <数量>] [--json <文件>]
```

参数：

| 参数 | 说明 |
| --- | --- |
| `--chat <chat_id>` | 必填；来自 `chats` 输出 |
| `--limit <数量>` | 最多读取 N 条消息；不指定时默认一直读取到没有更多消息 |
| `--json <文件>` | 将消息写入 JSON 数组；不指定时命令行输出结果 |

命令行输出：

| 字段 | 说明 |
| --- | --- |
| `message_id` | 消息 ID，下载时通过它指定具体消息 |
| `date` | 消息时间 |
| `type` | TDLib 原始类型名，例如 `messageVideo`、`messageText` |
| `file` | 文件名或文件信息 |
| `text` | 消息文本或视频说明 |

命令行输出示例：

| message_id | date | type | file | text |
| --- | --- | --- | --- | --- |
| `12345` | `2026-07-28 12:00` | `messageVideo` | `video.mp4` | `caption` |
| `12346` | `2026-07-28 12:03` | `messageText` | | `hello` |

示例：

```bash
./tg-tools messages --chat -1001234567890 --limit 50
./tg-tools messages --chat -1001234567890 --json messages.json
```

`messages --json messages.json` 输出示例：

```json
[
	{
		"message_id": 12345,
		"date": 1785235200,
		"date_text": "2026-07-28 12:00",
		"type": "messageVideo",
		"file_id": 654321,
		"file_name": "video.mp4",
		"mime_type": "video/mp4",
		"duration": 42,
		"width": 1920,
		"height": 1080,
		"text": "caption"
	}
]
```

### download

```bash
./tg-tools download --chat <chat_id> --message <message_id> [--out <路径>]
./tg-tools download --chat <chat_id> --messages <ID1,ID2,ID3> --out <目录>
```

参数：

| 参数 | 说明 |
| --- | --- |
| `--chat <chat_id>` | 必填；来自 `chats` 输出 |
| `--message <message_id>` | 下载单条消息中的视频 |
| `--messages <ID1,ID2,ID3>` | 批量下载多条消息中的视频，多个 ID 用英文逗号分隔 |
| `--out <路径>` | 输出目录或文件路径；默认是 `downloads` |

输出路径规则：

- 单条下载时，`--out` 可以是目录，也可以是完整文件路径
- 批量下载时，`--out` 必须是目录
- 如果部分消息下载失败，会打印对应的 `message_id`、`file_id` 和失败原因

示例：

```bash
./tg-tools download --chat -1001234567890 --message 12345 --out downloads
./tg-tools download --chat -1001234567890 --message 12345 --out downloads/video.mp4
./tg-tools download --chat -1001234567890 --messages 12345,12346 --out downloads
```

### upload

```bash
./tg-tools upload --chat <chat_id> --file <文件> [--caption <文本>]
./tg-tools upload --chat <chat_id> --json <文件>
```

参数：

| 参数 | 说明 |
| --- | --- |
| `--chat <chat_id>` | 必填；目标聊天或频道 ID |
| `--file <文件>` | 上传单个本地视频文件 |
| `--caption <文本>` | 单个文件的说明文字 |
| `--json <文件>` | 按 JSON 列表批量上传视频 |

示例：

```bash
./tg-tools upload --chat -1009876543210 --file downloads/video.mp4 --caption "saved locally"
./tg-tools upload --chat -1009876543210 --json uploads.json
```

`upload --json uploads.json` 输入格式如下，`name` 必填，`caption` 可选：

```json
[
	{"name": "downloads/a.mp4", "caption": "aaaa"},
	{"name": "downloads/b.mp4", "caption": "bbbb"}
]
```

## 更多帮助

命令行内置帮助会列出当前可用命令和参数：

```bash
./tg-tools help
./tg-tools help messages
./tg-tools help download
./tg-tools help upload
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
