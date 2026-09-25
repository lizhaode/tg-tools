# AGENTS.md

tg-tools 支持查看 Telegram 聊天消息、下载消息里的视频。

## 重要参考

- **Telegram 官方文档**：<https://core.telegram.org/>
- **TDLib 源码**：<https://github.com/tdlib/td>

## 代码编写原则

- 命名遵循 `.clang-tidy` 中的命名规则。

## 修改代码后必须执行 format

**每次修改完 `.cpp` / `.h` 文件后，都必须运行格式化，确认无报错后再交付。**

命令示例：

```bash
clang-format -i $(find src -name '*.cpp' -o -name '*.h')
```

注意：

- 只对本次修改涉及的文件运行即可，不要对 `vendor/`、`.tdlib-src/` 等第三方代码运行。
- 不要执行 `cmake --build`（编译太慢），构建验证由用户自行完成。

如果用户要执行 tidy，则

```bash
for f in $(find src -name '*.cpp'); do clang-tidy -p build -quiet "$f"; done
```
