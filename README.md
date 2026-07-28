# tg-tools

Telegram 视频命令行工具。基于 TDLib 原生 C++ 接口，可以登录账号、查看聊天和消息、下载消息里的视频、上传本地视频。

仅支持 Apple Silicon MacOS。

## 安装依赖

```bash
xcode-select --install
brew install cmake gperf openssl@3
```

## 构建

```bash
bash scripts/build.sh
```

构建产物在当前项目目录内，最终二进制为：

```bash
./tg-tools
```

## 配置

```bash
cp config/telegram.example.conf config/telegram.conf
```

然后编辑 `config/telegram.conf`，填入 Telegram `api_id` 和 `api_hash`。

如需代理：

```bash
export ALL_PROXY=http://127.0.0.1:7890
```

## 常用命令

```bash
./tg-tools login
./tg-tools chats --limit 50
./tg-tools messages --chat -1001234567890 --limit 50
./tg-tools messages --chat -1001234567890 --json messages.json
./tg-tools download --chat -1001234567890 --message 12345 --out downloads
./tg-tools download --chat -1001234567890 --message 12345 --out downloads/video.mp4
./tg-tools download --chat -1001234567890 --messages 12345,12346 --out downloads
./tg-tools upload --chat -1009876543210 --file downloads/video.mp4 --caption "saved locally"
./tg-tools upload --chat -1009876543210 --json uploads.json
```

单条下载时，`--out` 可以是目录，也可以是完整文件路径；批量下载时，`--out` 需要是目录。

更多参数说明看内置帮助：

```bash
./tg-tools help
./tg-tools help messages
./tg-tools help download
```

## 开发

项目使用 Google C++ 风格配置，规则见 `.clang-format` 和 `.clang-tidy`。

需要本地运行格式化和静态检查时再安装 `llvm`：

```bash
brew install llvm
```

## JSON 示例

`messages --json messages.json` 输出示例：

```json
[
	{
		"message_id": 12345,
		"date": 1785235200,
		"date_text": "2026-07-28 12:00",
		"type": "video",
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

`upload --json uploads.json` 输入示例，`name` 必填，`caption` 可选：

```json
[
	{"name": "downloads/a.mp4", "caption": "aaaa"},
	{"name": "downloads/b.mp4", "caption": "bbbb"}
]
```
