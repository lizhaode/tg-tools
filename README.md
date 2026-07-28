# tg-video-cli

C++17 命令行工具，使用 TDLib 原生 C++ 接口读取 Telegram 聊天消息、下载视频、上传视频。

支持环境：Linux x86_64、macOS arm64/M1。

## 依赖

Linux x86_64 / Ubuntu：

```bash
sudo apt update
sudo apt install -y git make g++ cmake gperf zlib1g-dev libssl-dev php-cli
```

macOS arm64/M1：

```bash
brew install git cmake gperf openssl@3
```

## 构建

在项目根目录运行：

```bash
bash scripts/build.sh
```

生成物只写在当前项目目录下：`.tdlib-src/`、`vendor/`、`build/`、`.local/`。

## 配置

```bash
cp config/telegram.example.conf config/telegram.conf
```

编辑 `config/telegram.conf`，填入 `api_id` 和 `api_hash`。配置项说明都在 conf 注释里。

## 代理

访问 Telegram 需要代理时，在运行程序前设置当前 shell 的代理环境变量：

```bash
export ALL_PROXY=http://127.0.0.1:78980
```

支持：`ALL_PROXY`、`all_proxy`、`HTTPS_PROXY`、`https_proxy`、`HTTP_PROXY`、`http_proxy`。代理值只支持 `http://host:port` 格式。

## 使用

```bash
./build/tg-video-cli login
./build/tg-video-cli chats
./build/tg-video-cli chats --limit 50
./build/tg-video-cli messages --chat -1001234567890
./build/tg-video-cli messages --chat -1001234567890 --limit 50
./build/tg-video-cli messages --chat -1001234567890 --json messages.json
./build/tg-video-cli download --chat -1001234567890 --message 12345 --out downloads
./build/tg-video-cli download --chat -1001234567890 --message 12345 --out downloads/video.mp4
./build/tg-video-cli download --chat -1001234567890 --messages 12345,12346,12347 --out downloads
./build/tg-video-cli upload --chat -1009876543210 --file downloads/video.mp4 --caption "saved locally"
./build/tg-video-cli upload --chat -1009876543210 --json uploads.json
```

`messages` 默认以表格输出：`message_id date type file text`。加 `--json FILE` 时输出 JSON 数组，包含 `message_id`、时间、类型、文件信息、尺寸/时长和文本等字段。表格里的 `file` 和 `text` 超过 120 字符会以 `....` 结尾。

`download` 会把文件复制到 `--out` 指定的位置，然后调用 TDLib `deleteFile` 清理 TDLib 文件缓存。单个 `--message` 时，`--out` 没有扩展名按目录处理，有扩展名按最终文件路径处理。批量 `--messages` 时，`--out` 必须是目录，固定同时下载 3 个；失败会打印 `message_id`、`file_id`、目标路径和原因。

`upload --json` 只读取标准 JSON 数组；每项包含 `name` 和可选 `caption`。上传失败会继续处理后续文件，最后打印失败文件和原因。

```json
[
	{"name": "downloads/a.mp4", "caption": "aaaa"},
	{"name": "downloads/b.mp4", "caption": "bbbb"}
]
```