# AGENTS.md

tg-tools 支持查看 Telegram 聊天消息、下载消息里的视频，以及将本地视频单个或批量上传到 Telegram。

## 重要参考

- **Telegram 官方文档**：<https://core.telegram.org/>
- **TDLib 源码**：<https://github.com/tdlib/td>

## 代码编写原则

- **保持简单**：优先写最简单、最直接的实现，禁止过度设计（不要提前抽象、不要引入不必要的类/模板/第三方依赖）。
- 遵循项目已有的代码风格与结构，新代码和同目录现有代码保持一致。
- 命名遵循 `.clang-tidy` 中的命名规则。
- 不引入新依赖除非确有必要；必要时先说明理由。

## 修改代码后必须执行 format 和 tidy

**每次修改完 `.cpp` / `.h` 文件后，都必须运行格式化和静态检查，确认无报错后再交付。**

命令示例：

```bash
clang-format -i $(find src -name '*.cpp' -o -name '*.h')
for f in $(find src -name '*.cpp'); do clang-tidy -p build -quiet "$f"; done
```

注意：

- 若 `build/compile_commands.json` 不存在，先执行 `bash scripts/build.sh` 生成。
- 只对本次修改涉及的文件运行即可，不要对 `vendor/`、`.tdlib-src/` 等第三方代码运行。
