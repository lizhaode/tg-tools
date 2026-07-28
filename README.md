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


编译输出

```bash
192:tg-video-cli lizhao$ bash scripts/build.sh 
-- The CXX compiler identification is AppleClang 21.0.0.21000101
-- The C compiler identification is AppleClang 21.0.0.21000101
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Could NOT find ccache (this is NOT an error)
-- Found OpenSSL: /opt/homebrew/opt/openssl@3/lib/libcrypto.dylib (found version "3.6.3")
-- Found OpenSSL: /opt/homebrew/opt/openssl@3/include /opt/homebrew/opt/openssl@3/lib/libssl.dylib;/opt/homebrew/opt/openssl@3/lib/libcrypto.dylib
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE
-- Performing Test HAVE_STD17
-- Performing Test HAVE_STD17 - Success
-- Performing Test HAVE_CXX_FLAG_WALL
-- Performing Test HAVE_CXX_FLAG_WALL - Success
-- Performing Test HAVE_CXX_FLAG_WEXTRA
-- Performing Test HAVE_CXX_FLAG_WEXTRA - Success
-- Performing Test HAVE_CXX_FLAG_FLTO_ODR_TYPE_MERGING
-- Performing Test HAVE_CXX_FLAG_FLTO_ODR_TYPE_MERGING - Failed
-- Performing Test HAVE_CXX_FLAG_QUNUSED_ARGUMENTS
-- Performing Test HAVE_CXX_FLAG_QUNUSED_ARGUMENTS - Success
-- Performing Test HAVE_CXX_FLAG_WALLOC_ZERO
-- Performing Test HAVE_CXX_FLAG_WALLOC_ZERO - Failed
-- Performing Test HAVE_CXX_FLAG_WCXX17_COMPAT_PEDANTIC
-- Performing Test HAVE_CXX_FLAG_WCXX17_COMPAT_PEDANTIC - Success
-- Performing Test HAVE_CXX_FLAG_WCAST_QUAL
-- Performing Test HAVE_CXX_FLAG_WCAST_QUAL - Success
-- Performing Test HAVE_CXX_FLAG_WCONVERSION
-- Performing Test HAVE_CXX_FLAG_WCONVERSION - Success
-- Performing Test HAVE_CXX_FLAG_WDEPRECATED
-- Performing Test HAVE_CXX_FLAG_WDEPRECATED - Success
-- Performing Test HAVE_CXX_FLAG_WDUPLICATED_BRANCHES
-- Performing Test HAVE_CXX_FLAG_WDUPLICATED_BRANCHES - Failed
-- Performing Test HAVE_CXX_FLAG_WDUPLICATED_COND
-- Performing Test HAVE_CXX_FLAG_WDUPLICATED_COND - Failed
-- Performing Test HAVE_CXX_FLAG_WIMPLICIT_FALLTHROUGH_2
-- Performing Test HAVE_CXX_FLAG_WIMPLICIT_FALLTHROUGH_2 - Failed
-- Performing Test HAVE_CXX_FLAG_WLOGICAL_OP
-- Performing Test HAVE_CXX_FLAG_WLOGICAL_OP - Failed
-- Performing Test HAVE_CXX_FLAG_WPSABI
-- Performing Test HAVE_CXX_FLAG_WPSABI - Success
-- Performing Test HAVE_CXX_FLAG_WSIGN_CONVERSION
-- Performing Test HAVE_CXX_FLAG_WSIGN_CONVERSION - Success
-- Performing Test HAVE_CXX_FLAG_WTAUTOLOGICAL_COMPARE
-- Performing Test HAVE_CXX_FLAG_WTAUTOLOGICAL_COMPARE - Success
-- Performing Test HAVE_CXX_FLAG_WUNKNOWN_WARNING_OPTION
-- Performing Test HAVE_CXX_FLAG_WUNKNOWN_WARNING_OPTION - Success
-- Performing Test HAVE_CXX_FLAG_WUNUSED_COMMAND_LINE_ARGUMENT
-- Performing Test HAVE_CXX_FLAG_WUNUSED_COMMAND_LINE_ARGUMENT - Success
-- Performing Test HAVE_CXX_FLAG_WUNUSED_PARAMETER
-- Performing Test HAVE_CXX_FLAG_WUNUSED_PARAMETER - Success
-- Performing Test HAVE_CXX_FLAG_WNON_VIRTUAL_DTOR
-- Performing Test HAVE_CXX_FLAG_WNON_VIRTUAL_DTOR - Success
-- Performing Test HAVE_CXX_FLAG_WODR
-- Performing Test HAVE_CXX_FLAG_WODR - Success
-- Performing Test HAVE_CXX_FLAG_WPOINTER_ARITH
-- Performing Test HAVE_CXX_FLAG_WPOINTER_ARITH - Success
-- Performing Test HAVE_CXX_FLAG_WSIGN_COMPARE
-- Performing Test HAVE_CXX_FLAG_WSIGN_COMPARE - Success
-- Performing Test HAVE_CXX_FLAG_WUNUSED_MEMBER_FUNCTION
-- Performing Test HAVE_CXX_FLAG_WUNUSED_MEMBER_FUNCTION - Success
-- Performing Test HAVE_CXX_FLAG_WUNUSED_PRIVATE_FIELD
-- Performing Test HAVE_CXX_FLAG_WUNUSED_PRIVATE_FIELD - Success
-- Performing Test HAVE_CXX_FLAG_WVLA
-- Performing Test HAVE_CXX_FLAG_WVLA - Success
-- Git state: 022d60202e446ad1287b9fb68e687c8a0760788b
-- Found ZLIB: /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib/libz.tbd (found version "1.2.12")
-- Found ZLIB: /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib/libz.tbd
-- Performing Test ATOMICS_FOUND
-- Performing Test ATOMICS_FOUND - Success
-- Performing Test COMPILER_HAS_HIDDEN_VISIBILITY
-- Performing Test COMPILER_HAS_HIDDEN_VISIBILITY - Success
-- Performing Test COMPILER_HAS_HIDDEN_INLINE_VISIBILITY
-- Performing Test COMPILER_HAS_HIDDEN_INLINE_VISIBILITY - Success
-- Performing Test COMPILER_HAS_DEPRECATED_ATTR
-- Performing Test COMPILER_HAS_DEPRECATED_ATTR - Success
-- Performing Test GNU_READLINE_FOUND
-- Performing Test GNU_READLINE_FOUND - Success
-- Found Readline: /opt/homebrew/opt/readline/include /opt/homebrew/opt/readline/lib/libreadline.dylib
-- Performing Test USABLE_READLINE_FOUND
-- Performing Test USABLE_READLINE_FOUND - Success
-- Configuring done (5.1s)
-- Generating done (0.3s)
CMake Warning (unused-cli):
  Manually-specified variables were not used by the project:

    CMAKE_EXPORT_NO_PACKAGE_REGISTRY


-- Build files have been written to: /Users/lizhao/Documents/Airport-Config/tg-video-cli/.tdlib-src/build-tdlib
[  1%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_config.cpp.o
[  1%] Building C object sqlite/CMakeFiles/tdsqlite.dir/sqlite/sqlite3.c.o
[  1%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_file_outputer.cpp.o
[  1%] Building C object td/generate/tl-parser/CMakeFiles/tl-parser.dir/crc32.c.o
[  1%] Building C object td/generate/tl-parser/CMakeFiles/tl-parser.dir/tlc.c.o
[  2%] Building C object td/generate/tl-parser/CMakeFiles/tl-parser.dir/tl-parser.c.o
[  2%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_core.cpp.o
[  2%] Building CXX object tdutils/generate/CMakeFiles/generate_mime_types_gperf.dir/generate_mime_types_gperf.cpp.o
[  2%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_file_utils.cpp.o
[  2%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_generate.cpp.o
[  2%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_outputer.cpp.o
[  2%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_string_outputer.cpp.o
[  2%] Building CXX object tdtl/CMakeFiles/tdtl.dir/td/tl/tl_writer.cpp.o
[  2%] Linking C executable tl-parser
[  2%] Built target tl-parser
[  2%] Generate TLO files
[  2%] Linking CXX executable generate_mime_types_gperf
[  2%] Built target generate_mime_types_gperf
[  2%] Generating /Users/lizhao/Documents/Airport-Config/tg-video-cli/.tdlib-src/td/tdutils/generate/auto/mime_type_to_extension.gperf, /Users/lizhao/Documents/Airport-Config/tg-video-cli/.tdlib-src/td/tdutils/generate/auto/extension_to_mime_type.gperf
[  3%] Linking CXX static library libtdtl.a
[  3%] Built target tdtl
[  3%] Building CXX object td/generate/CMakeFiles/generate_c.dir/generate_c.cpp.o
[  4%] Building CXX object td/generate/CMakeFiles/td_generate_java_api.dir/tl_writer_java.cpp.o
[  4%] Building CXX object td/generate/CMakeFiles/td_generate_java_api.dir/generate_java.cpp.o
[  4%] Building CXX object td/generate/CMakeFiles/tl_writer_cpp.dir/tl_writer_cpp.cpp.o
[  5%] Building CXX object td/generate/CMakeFiles/tl_writer_cpp.dir/tl_writer_h.cpp.o
[  5%] Built target tl_generate_tlo
[  5%] Building CXX object td/generate/CMakeFiles/tl_writer_cpp.dir/tl_writer_hpp.cpp.o
Extension "wmz" matches more than one type
Extension "sub" matches more than one type
[  5%] Generating /Users/lizhao/Documents/Airport-Config/tg-video-cli/.tdlib-src/td/tdutils/generate/auto/mime_type_to_extension.cpp
[  5%] Building CXX object td/generate/CMakeFiles/tl_writer_cpp.dir/tl_writer_jni_cpp.cpp.o
[  5%] Building CXX object td/generate/CMakeFiles/tl_writer_cpp.dir/tl_writer_jni_h.cpp.o
[  5%] Linking CXX executable td_generate_java_api
[  5%] Built target td_generate_java_api
[  5%] Building CXX object td/generate/CMakeFiles/tl_writer_cpp.dir/tl_writer_td.cpp.o
[  5%] Generating /Users/lizhao/Documents/Airport-Config/tg-video-cli/.tdlib-src/td/tdutils/generate/auto/extension_to_mime_type.cpp
[  5%] Linking CXX static library libtl_writer_cpp.a
[  5%] Linking CXX executable generate_c
[  5%] Built target tl_writer_cpp
[  5%] Built target generate_c
[  6%] Building CXX object td/generate/CMakeFiles/generate_mtproto.dir/generate_mtproto.cpp.o
[  7%] Building CXX object td/generate/CMakeFiles/generate_common.dir/generate_common.cpp.o
[  7%] Linking CXX executable generate_mtproto
[  7%] Linking CXX executable generate_common
ld: warning: ignoring duplicate libraries: '../../tdtl/libtdtl.a'
ld: warning: ignoring duplicate libraries: '../../tdtl/libtdtl.a'
[  7%] Built target generate_mtproto
[  7%] Built target generate_common
[  7%] Generate MTProto API source files
[  7%] Generate common TL source files
Write file td/mtproto/mtproto_api.cpp
Write file td/mtproto/mtproto_api.h
Write file td/mtproto/mtproto_api.hpp
[  7%] Built target tl_generate_mtproto
Write file td/telegram/telegram_api_0.cpp
Write file td/telegram/telegram_api_1.cpp
Write file td/telegram/telegram_api_2.cpp
Write file td/telegram/telegram_api_3.cpp
Write file td/telegram/telegram_api_4.cpp
Write file td/telegram/telegram_api_5.cpp
Write file td/telegram/telegram_api_6.cpp
Write file td/telegram/telegram_api_7.cpp
Write file td/telegram/telegram_api_8.cpp
Write file td/telegram/telegram_api_9.cpp
Write file td/telegram/telegram_api.h
Write file td/telegram/telegram_api.hpp
Write file td/telegram/secret_api.cpp
Write file td/telegram/secret_api.h
Write file td/telegram/secret_api.hpp
Write file td/telegram/e2e_api.cpp
Write file td/telegram/e2e_api.h
Write file td/telegram/e2e_api.hpp
Write file td/telegram/td_api_0.cpp
Write file td/telegram/td_api_1.cpp
Write file td/telegram/td_api_2.cpp
Write file td/telegram/td_api_3.cpp
Write file td/telegram/td_api_4.cpp
Write file td/telegram/td_api_5.cpp
Write file td/telegram/td_api_6.cpp
Write file td/telegram/td_api_7.cpp
Write file td/telegram/td_api_8.cpp
Write file td/telegram/td_api_9.cpp
Write file td/telegram/td_api.h
Write file td/telegram/td_api.hpp
[  7%] Built target tl_generate_common
[  7%] Built target tdmime_auto
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/MemoryMapping.cpp.o
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/IPAddress.cpp.o
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/Clocks.cpp.o
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/FileFd.cpp.o
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/path.cpp.o
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/PollFlags.cpp.o
[  7%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/platform.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/rlimit.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/ServerSocketFd.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/signals.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/sleep.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/SocketFd.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/stacktrace.cpp.o
[  8%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/Stat.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/StdStreams.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/thread_local.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/uname.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/UdpSocketFd.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/user.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/wstring_convert.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/Epoll.cpp.o
[  9%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/EventFdBsd.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/EventFdLinux.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/EventFdWindows.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/Iocp.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/KQueue.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/NativeFd.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/Poll.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/Select.cpp.o
[ 10%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/ThreadIdGuard.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/ThreadPthread.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/port/detail/WineventPoll.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/generate/auto/mime_type_to_extension.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/generate/auto/extension_to_mime_type.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/AsyncFileLog.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/base64.cpp.o
[ 11%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/BigNum.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/buffer.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/BufferedUdp.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/check.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/crypto.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Ed25519.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/emoji.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/ExitGuard.cpp.o
[ 12%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/FileLog.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/filesystem.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/find_boundary.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/FlatHashTable.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/FloodControlGlobal.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Gzip.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/GzipByteFlow.cpp.o
[ 13%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Hints.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/HttpDate.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/HttpUrl.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/JsonBuilder.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/logging.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/misc.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/MpmcQueue.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/OptionParser.cpp.o
[ 14%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/PathView.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Random.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/SharedSlice.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Slice.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/StackAllocator.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Status.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/StringBuilder.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/tests.cpp.o
[ 15%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Time.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/Timer.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/tl_parsers.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/translit.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/TsCerr.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/TsFileLog.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/TsLog.cpp.o
[ 17%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/unicode.cpp.o
[ 18%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/utf8.cpp.o
[ 18%] Building CXX object tdutils/CMakeFiles/tdutils.dir/td/utils/MimeType.cpp.o
[ 18%] Linking C static library libtdsqlite.a
[ 18%] Linking CXX static library libtdutils.a
[ 18%] Built target tdsqlite
[ 18%] Built target tdutils
[ 18%] Building CXX object benchmark/CMakeFiles/rmdir.dir/rmdir.cpp.o
[ 18%] Building CXX object td/generate/CMakeFiles/generate_json.dir/generate_json.cpp.o
[ 18%] Building CXX object benchmark/CMakeFiles/bench_crypto.dir/bench_crypto.cpp.o
[ 18%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/bip39.cpp.o
[ 18%] Building CXX object tdactor/CMakeFiles/tdactor.dir/td/actor/ConcurrentScheduler.cpp.o
[ 19%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_0.cpp.o
[ 19%] Building CXX object CMakeFiles/memprof.dir/memprof/memprof.cpp.o
[ 19%] Building CXX object benchmark/CMakeFiles/check_tls.dir/check_tls.cpp.o
[ 19%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/BitString.cpp.o
[ 19%] Linking CXX static library libmemprof.a
[ 19%] Building CXX object td/generate/CMakeFiles/generate_json.dir/tl_json_converter.cpp.o
[ 19%] Linking CXX executable rmdir
[ 19%] Built target memprof
[ 19%] Building CXX object benchmark/CMakeFiles/bench_empty.dir/bench_empty.cpp.o
[ 19%] Built target rmdir
[ 19%] Building CXX object benchmark/CMakeFiles/bench_log.dir/bench_log.cpp.o
[ 20%] Linking CXX executable bench_empty
[ 20%] Built target bench_empty
[ 20%] Building CXX object tdactor/CMakeFiles/tdactor.dir/td/actor/impl/Scheduler.cpp.o
[ 20%] Linking CXX executable bench_crypto
[ 20%] Building CXX object benchmark/CMakeFiles/bench_queue.dir/bench_queue.cpp.o
[ 20%] Linking CXX executable check_tls
[ 20%] Built target bench_crypto
[ 20%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_1.cpp.o
[ 20%] Built target check_tls
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/Blockchain.cpp.o
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/Call.cpp.o
[ 21%] Linking CXX executable bench_log
[ 21%] Built target bench_log
[ 21%] Building CXX object tdactor/CMakeFiles/tdactor.dir/td/actor/MultiPromise.cpp.o
[ 21%] Linking CXX executable bench_queue
[ 21%] Built target bench_queue
[ 21%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_2.cpp.o
[ 21%] Building CXX object tdactor/CMakeFiles/tdactor.dir/td/actor/MultiTimeout.cpp.o
[ 21%] Linking CXX executable generate_json
[ 21%] Built target generate_json
[ 21%] Generate JSON TL source files
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/CheckSharedSecret.cpp.o
[ 21%] Built target tl_generate_json
[ 21%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_3.cpp.o
[ 21%] Linking CXX static library libtdactor.a
[ 21%] Built target tdactor
[ 21%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/GetHostByNameActor.cpp.o
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/DecryptedKey.cpp.o
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/e2e_api.cpp.o
[ 21%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_4.cpp.o
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/EncryptedKey.cpp.o
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/EncryptedStorage.cpp.o
[ 21%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_5.cpp.o
[ 21%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpChunkedByteFlow.cpp.o
[ 21%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/Keys.cpp.o
[ 21%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpConnectionBase.cpp.o
[ 22%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/MessageEncryption.cpp.o
[ 22%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_6.cpp.o
[ 22%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/Mnemonic.cpp.o
[ 22%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/QRHandshake.cpp.o
[ 22%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpContentLengthByteFlow.cpp.o
[ 23%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_7.cpp.o
[ 23%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpFile.cpp.o
[ 23%] Building CXX object tde2e/CMakeFiles/tde2e.dir/td/e2e/Trie.cpp.o
[ 23%] Building CXX object tde2e/CMakeFiles/tde2e.dir/__/td/generate/auto/td/telegram/e2e_api.cpp.o
[ 24%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/binlog/Binlog.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpHeaderCreator.cpp.o
[ 25%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_8.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpInboundConnection.cpp.o
[ 25%] Building CXX object tdactor/CMakeFiles/example.dir/example/example.cpp.o
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/binlog/BinlogEvent.cpp.o
[ 25%] Building CXX object CMakeFiles/tdapi.dir/td/generate/auto/td/telegram/td_api_9.cpp.o
[ 25%] Linking CXX static library libtde2e.a
[ 25%] Built target tde2e
[ 25%] Building CXX object benchmark/CMakeFiles/bench_actor.dir/bench_actor.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpOutboundConnection.cpp.o
[ 25%] Linking CXX executable example
[ 25%] Built target example
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/binlog/ConcurrentBinlog.cpp.o
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/binlog/detail/BinlogEventsBuffer.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpProxy.cpp.o
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/binlog/detail/BinlogEventsProcessor.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpQuery.cpp.o
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/detail/RawSqliteDb.cpp.o
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/SqliteConnectionSafe.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/HttpReader.cpp.o
[ 25%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/Socks5.cpp.o
[ 25%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/SqliteDb.cpp.o
[ 26%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/SqliteKeyValue.cpp.o
[ 26%] Linking CXX executable bench_actor
[ 26%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/SqliteKeyValueAsync.cpp.o
[ 26%] Built target bench_actor
[ 26%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/SslCtx.cpp.o
[ 26%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/SqliteStatement.cpp.o
[ 26%] Building CXX object tddb/CMakeFiles/tddb.dir/td/db/TQueue.cpp.o
[ 27%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/SslStream.cpp.o
[ 27%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/TcpListener.cpp.o
[ 27%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/TransparentProxy.cpp.o
[ 27%] Building CXX object tdnet/CMakeFiles/tdnet.dir/td/net/Wget.cpp.o
[ 27%] Linking CXX static library libtdapi.a
[ 27%] Built target tdapi
[ 27%] Linking CXX static library libtddb.a
[ 27%] Built target tddb
[ 27%] Building CXX object benchmark/CMakeFiles/bench_db.dir/bench_db.cpp.o
[ 27%] Building CXX object tddb/CMakeFiles/binlog_dump.dir/td/db/binlog/binlog_dump.cpp.o
[ 27%] Linking CXX static library libtdnet.a
[ 27%] Built target tdnet
[ 27%] Building CXX object benchmark/CMakeFiles/bench_http_server_cheat.dir/bench_http_server_cheat.cpp.o
[ 27%] Building CXX object benchmark/CMakeFiles/bench_http.dir/bench_http.cpp.o
[ 27%] Building CXX object benchmark/CMakeFiles/bench_http_server_fast.dir/bench_http_server_fast.cpp.o
[ 27%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/AuthData.cpp.o
[ 27%] Building CXX object benchmark/CMakeFiles/bench_http_server.dir/bench_http_server.cpp.o
[ 27%] Building CXX object benchmark/CMakeFiles/bench_http_reader.dir/bench_http_reader.cpp.o
[ 27%] Linking CXX executable binlog_dump
[ 27%] Built target binlog_dump
[ 27%] Building CXX object benchmark/CMakeFiles/wget.dir/wget.cpp.o
[ 27%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/ConnectionManager.cpp.o
[ 27%] Linking CXX executable bench_http_reader
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 27%] Linking CXX executable bench_http_server_cheat
[ 27%] Built target bench_http_reader
[ 27%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/DhHandshake.cpp.o
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 27%] Built target bench_http_server_cheat
[ 28%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/Handshake.cpp.o
[ 28%] Linking CXX executable bench_http
[ 29%] Linking CXX executable bench_http_server
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 29%] Built target bench_http
[ 29%] Built target bench_http_server
[ 29%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/HandshakeActor.cpp.o
[ 29%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/HttpTransport.cpp.o
[ 29%] Linking CXX executable bench_http_server_fast
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 29%] Built target bench_http_server_fast
[ 29%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/IStreamTransport.cpp.o
[ 29%] Linking CXX executable bench_db
ld: warning: ignoring duplicate libraries: '../tdactor/libtdactor.a', '../tdutils/libtdutils.a'
[ 29%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/KDF.cpp.o
[ 29%] Built target bench_db
[ 29%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/Ping.cpp.o
[ 29%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/PingConnection.cpp.o
[ 30%] Linking CXX executable wget
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 30%] Built target wget
[ 30%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/ProxySecret.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/RawConnection.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/RSA.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/SessionConnection.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/TcpTransport.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/TlsInit.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/TlsReaderByteFlow.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/Transport.cpp.o
[ 31%] Building CXX object CMakeFiles/tdmtproto.dir/td/mtproto/utils.cpp.o
[ 32%] Building CXX object CMakeFiles/tdmtproto.dir/td/generate/auto/td/mtproto/mtproto_api.cpp.o
[ 32%] Linking CXX static library libtdmtproto.a
[ 32%] Built target tdmtproto
[ 32%] Building CXX object benchmark/CMakeFiles/bench_handshake.dir/bench_handshake.cpp.o
[ 32%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AccountManager.cpp.o
[ 32%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ActiveStoryState.cpp.o
[ 32%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AffiliateType.cpp.o
[ 32%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AiComposeToneExample.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AlarmManager.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AiComposeTone.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AgeVerificationParameters.cpp.o
[ 34%] Linking CXX executable bench_handshake
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 34%] Built target bench_handshake
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AnimationsManager.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Application.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AttachMenuManager.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AuctionBidLevel.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AudiosManager.cpp.o
[ 34%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AuthManager.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AutoDownloadSettings.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/AutosaveManager.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BackgroundInfo.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BackgroundManager.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BackgroundType.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BaseTheme.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Birthdate.cpp.o
[ 35%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BoostManager.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotAccessSettings.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotCommand.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotCommandScope.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotInfoManager.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotMenuButton.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotQueries.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotRecommendationManager.cpp.o
[ 36%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotVerification.cpp.o
[ 37%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BotVerifierSettings.cpp.o
[ 37%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessAwayMessage.cpp.o
[ 37%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessAwayMessageSchedule.cpp.o
[ 37%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessBotManageBar.cpp.o
[ 37%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessBotRights.cpp.o
[ 37%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessChatLink.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessConnectionManager.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessConnectedBot.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessGreetingMessage.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessInfo.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessIntro.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessRecipients.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessManager.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/BusinessWorkHours.cpp.o
[ 38%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CallActor.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CallbackQueriesManager.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CallDiscardReason.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CallManager.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ChannelParticipantFilter.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ChannelRecommendationManager.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ChatManager.cpp.o
[ 39%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ChatReactions.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ChatTheme.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ClientActor.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CommonDialogManager.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CommunityManager.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ConfigManager.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ConnectionState.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ConnectionStateManager.cpp.o
[ 40%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Contact.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CountryInfoManager.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/CurrencyAmount.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DelayDispatcher.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Dependencies.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DeviceTokenManager.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DhCache.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogAction.cpp.o
[ 41%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogActionBar.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogActionManager.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogAdministrator.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogDb.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogEventLog.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogFilter.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogFilterInviteLink.cpp.o
[ 42%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogFilterManager.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogId.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogInviteLink.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogInviteLinkManager.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogListId.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogLocation.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogManager.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogNotificationSettings.cpp.o
[ 43%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogParticipant.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogParticipantFilter.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogParticipantManager.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogPhoto.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DialogSource.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DiffText.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Dimensions.cpp.o
[ 44%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DisallowedGiftsSettings.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Document.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DocumentsManager.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DownloadManager.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DownloadManagerCallback.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DraftMessage.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/DraftMessageManager.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/EmailVerification.cpp.o
[ 45%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/EmojiGameInfo.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/EmojiGroup.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/EmojiGroupType.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/EmojiStatus.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/FactCheck.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/FileReferenceManager.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileBitmask.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileDb.cpp.o
[ 46%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileDownloader.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileDownloadManager.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileEncryptionKey.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileFromBytes.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileGcParameters.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileGcWorker.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileGenerateManager.cpp.o
[ 47%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileHashUploader.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileLoaderUtils.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileLoadManager.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileManager.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileStats.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileStatsWorker.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileType.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileUploader.cpp.o
[ 48%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileUploadId.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/FileUploadManager.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/PartsManager.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/ResourceManager.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/files/ResourceState.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/FormattedDate.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ForumTopic.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ForumTopicEditedData.cpp.o
[ 50%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ForumTopicIcon.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ForumTopicInfo.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ForumTopicManager.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Game.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GameManager.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GiveawayParameters.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Global.cpp.o
[ 51%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GlobalPrivacySettings.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallJoinParameters.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallManager.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallMessage.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallMessageLimit.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallParticipant.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallParticipantOrder.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GroupCallVideoPayload.cpp.o
[ 52%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/HashtagHints.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InlineMessageManager.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InlineQueriesManager.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputBusinessChatLink.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputCallId.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputDialogId.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputGroupCall.cpp.o
[ 53%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputGroupCallId.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputInvoice.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/InputMessageText.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/JoinChatBotResult.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/JsonValue.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/KeyboardButtonStyle.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/LabeledPricePart.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/LanguagePackManager.cpp.o
[ 54%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/LinkManager.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Location.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/logevent/LogEventHelper.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Logging.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MediaArea.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MediaAreaCoordinates.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageContent.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageContentType.cpp.o
[ 55%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageCopyOptions.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageCover.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageDb.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageEntity.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageExtendedMedia.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageForwardInfo.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageId.cpp.o
[ 56%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageImportManager.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageInputReplyTo.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageOrigin.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageQueryManager.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageQuote.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageReaction.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageReactor.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageReplyHeader.cpp.o
[ 57%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageReplyInfo.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageSearchFilter.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageSearchOffset.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageSelfDestructType.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageSender.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageSendOptions.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessagesInfo.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessagesManager.cpp.o
[ 58%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageSource.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageThreadDb.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageTopic.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageTtl.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MessageViewer.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/misc.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/MissingInvitee.cpp.o
[ 59%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/AuthDataShared.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/ConnectionCreator.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/DcAuthManager.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/DcOptionsSet.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/MtprotoHeader.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetActor.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetQuery.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetQueryCreator.cpp.o
[ 60%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetQueryDelayer.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetQueryDispatcher.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetQueryStats.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetQueryVerifier.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/NetStatsManager.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/Proxy.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/ProxyChecker.cpp.o
[ 61%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/PublicRsaKeySharedCdn.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/PublicRsaKeySharedMain.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/PublicRsaKeyWatchdog.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/Session.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/SessionMultiProxy.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/net/SessionProxy.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NewPasswordState.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationGroupInfo.cpp.o
[ 62%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationGroupType.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationManager.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationSettingsManager.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationSettingsScope.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationSound.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/NotificationType.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/OnlineManager.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/OptionManager.cpp.o
[ 63%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/OrderedMessage.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/OrderInfo.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Outline.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PaidReactionType.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Passkey.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PasswordManager.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Payments.cpp.o
[ 64%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PeerColor.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PeerColorCollectible.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PeopleNearbyManager.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PhoneNumberManager.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Photo.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PhotoSize.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PhotoSizeSource.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PollManager.cpp.o
[ 65%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PollOption.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Premium.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PremiumGiftOption.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PrivacyManager.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ProfileTab.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/PromoDataManager.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/QueryCombiner.cpp.o
[ 67%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/QueryMerger.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/QuickReplyManager.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReactionListType.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReactionManager.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReactionNotificationSettings.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReactionNotificationsFrom.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReactionType.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/RecentDialogList.cpp.o
[ 68%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReferralProgramInfo.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReferralProgramManager.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReferralProgramParameters.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReferralProgramSortOrder.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/RepliedMessageInfo.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReplyMarkup.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ReportReason.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/RequestedDialogType.cpp.o
[ 69%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Requests.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/RestrictionReason.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/RichMessage.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/RichMessageMedia.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SavedMessagesManager.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SavedMessagesTopicId.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ScopeNotificationSettings.cpp.o
[ 70%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SearchPostsFlood.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecretChatActor.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecretChatDb.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecretChatsManager.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecretInputMedia.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecureManager.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecureStorage.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SecureValue.cpp.o
[ 71%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SendCodeHelper.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SentEmailCode.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SequenceDispatcher.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SharedDialog.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SpecialStickerSetType.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SponsoredMessageManager.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarAmount.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGift.cpp.o
[ 72%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftAttribute.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftAttributeId.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftAttributeRarity.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftAuctionRound.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftAuctionState.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftAuctionUserState.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftBackground.cpp.o
[ 73%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftCollection.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftId.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftManager.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftResalePrice.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarGiftSettings.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarManager.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarRating.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarSubscription.cpp.o
[ 74%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StarSubscriptionPricing.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StateManager.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StatisticsManager.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickerFormat.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickerListType.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickerMaskPosition.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickerPhotoSize.cpp.o
[ 75%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickerSetId.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickersManager.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StickerType.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StorageManager.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryAlbum.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryContentType.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryContent.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryDb.cpp.o
[ 76%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryForwardInfo.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryInteractionInfo.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryManager.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryStealthMode.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/StoryViewer.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SuggestedAction.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SuggestedActionManager.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SuggestedPost.cpp.o
[ 77%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SuggestedPostPrice.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Support.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/SynchronousRequests.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TargetDialogTypes.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Td.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TdDb.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TempPasswordState.cpp.o
[ 78%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TermsOfService.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TermsOfServiceManager.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ThemeManager.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ThemeSettings.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TimeZoneManager.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ToDoCompletion.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ToDoItem.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/ToDoList.cpp.o
[ 79%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TonAmount.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TopDialogCategory.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TopDialogManager.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TranscriptionInfo.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TranscriptionManager.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/TranslationManager.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/UpdatesManager.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/UserId.cpp.o
[ 80%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/UserManager.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Usernames.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/UserPrivacySetting.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/UserPrivacySettingRule.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/UserStarGift.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/Venue.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/VerificationStatus.cpp.o
[ 81%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/VideoNotesManager.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/VideosManager.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/VoiceNotesManager.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebApp.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebAppManager.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebAppOpenParameters.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebBrowserManager.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebBrowserSettings.cpp.o
[ 82%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebDomainException.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebPageBlock.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/WebPagesManager.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_0.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_1.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_2.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_3.cpp.o
[ 84%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_4.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_5.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_6.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_7.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_8.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/telegram_api_9.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/generate/auto/td/telegram/secret_api.cpp.o
[ 85%] Building CXX object CMakeFiles/tdcore.dir/td/telegram/GitCommitHash.cpp.o
[ 85%] Linking CXX static library libtdcore.a
[ 85%] Built target tdcore
[ 85%] Building CXX object CMakeFiles/tdclient.dir/td/telegram/Client.cpp.o
[ 85%] Building CXX object benchmark/CMakeFiles/bench_tddb.dir/bench_tddb.cpp.o
[ 87%] Building CXX object CMakeFiles/tdclient.dir/td/telegram/Log.cpp.o
[ 87%] Building CXX object benchmark/CMakeFiles/bench_misc.dir/bench_misc.cpp.o
[ 87%] Linking CXX executable bench_tddb
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 87%] Built target bench_tddb
[ 87%] Linking CXX static library libtdclient.a
[ 87%] Built target tdclient
[ 87%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_1.cpp.o
[ 87%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_2.cpp.o
[ 87%] Building CXX object CMakeFiles/tg_cli.dir/td/telegram/cli.cpp.o
[ 88%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_0.cpp.o
[ 88%] Building CXX object benchmark/CMakeFiles/check_proxy.dir/check_proxy.cpp.o
[ 89%] Building CXX object test/CMakeFiles/run_all_tests.dir/main.cpp.o
[ 89%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_0.cpp.o
[ 89%] Linking CXX executable bench_misc
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 89%] Built target bench_misc
[ 89%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_3.cpp.o
[ 89%] Building CXX object test/CMakeFiles/run_all_tests.dir/country_info.cpp.o
[ 89%] Linking CXX executable check_proxy
ld: warning: ignoring duplicate libraries: '../tdutils/libtdutils.a'
[ 89%] Built target check_proxy
[ 89%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_1.cpp.o
[ 89%] Building CXX object test/CMakeFiles/run_all_tests.dir/db.cpp.o
[ 89%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_4.cpp.o
[ 89%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_2.cpp.o
[ 89%] Building CXX object test/CMakeFiles/run_all_tests.dir/http.cpp.o
[ 89%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_5.cpp.o
[ 90%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_6.cpp.o
[ 90%] Building CXX object test/CMakeFiles/run_all_tests.dir/link.cpp.o
[ 90%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_3.cpp.o
[ 90%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_7.cpp.o
[ 90%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_4.cpp.o
[ 90%] Building CXX object test/CMakeFiles/run_all_tests.dir/message_entities.cpp.o
[ 90%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_5.cpp.o
[ 90%] Building CXX object test/CMakeFiles/run_all_tests.dir/mtproto.cpp.o
[ 90%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_8.cpp.o
[ 91%] Building CXX object test/CMakeFiles/run_all_tests.dir/poll.cpp.o
[ 92%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_6.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/query_merger.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/secret.cpp.o
[ 92%] Building CXX object CMakeFiles/tdjson_private.dir/td/generate/auto/td/telegram/td_api_json_9.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/secure_storage.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/set_with_position.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/string_cleaning.cpp.o
[ 92%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_7.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/tdclient.cpp.o
[ 92%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_8.cpp.o
[ 92%] Building CXX object test/CMakeFiles/run_all_tests.dir/tqueue.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/data.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/bitmask.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/buffer.cpp.o
[ 93%] Building CXX object CMakeFiles/tg_cli.dir/td/generate/auto/td/telegram/td_api_json_9.cpp.o
[ 93%] Building CXX object CMakeFiles/tdjson_private.dir/td/telegram/ClientJson.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/ChainScheduler.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/ConcurrentHashMap.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/crypto.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/emoji.cpp.o
[ 93%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/Enumerator.cpp.o
[ 93%] Linking CXX static library libtdjson_private.a
[ 93%] Built target tdjson_private
[ 93%] Building CXX object CMakeFiles/tdjson.dir/td/telegram/td_json_client.cpp.o
[ 94%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/EpochBasedMemoryReclamation.cpp.o
[ 94%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/filesystem.cpp.o
[ 94%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/gzip.cpp.o
[ 94%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/HazardPointers.cpp.o
[ 94%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/HashSet.cpp.o
[ 94%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/heap.cpp.o
[ 94%] Building CXX object CMakeFiles/tdjson.dir/td/telegram/td_log.cpp.o
[ 95%] Linking CXX shared library libtdjson.dylib
[ 95%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/HttpUrl.cpp.o
[ 95%] Built target tdjson
[ 95%] Building CXX object CMakeFiles/tdjson_static.dir/td/telegram/td_json_client.cpp.o
[ 96%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/json.cpp.o
[ 96%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/List.cpp.o
[ 96%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/log.cpp.o
[ 96%] Building CXX object CMakeFiles/tdjson_static.dir/td/telegram/td_log.cpp.o
[ 96%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/misc.cpp.o
[ 97%] Linking CXX static library libtdjson_static.a
[ 97%] Built target tdjson_static
[ 97%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/MpmcQueue.cpp.o
[ 97%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/MpmcWaiter.cpp.o
[ 97%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/MpscLinkQueue.cpp.o
[ 97%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/OptionParser.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/OrderedEventsProcessor.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/port.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/pq.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/SharedObjectPool.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/SharedSlice.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/StealingQueue.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/variant.cpp.o
[ 98%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/WaitFreeHashMap.cpp.o
[ 98%] Linking CXX executable tg_cli
[100%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/WaitFreeHashSet.cpp.o
[100%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdutils/test/WaitFreeVector.cpp.o
[100%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdactor/test/actors_main.cpp.o
[100%] Built target tg_cli
[100%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdactor/test/actors_simple.cpp.o
[100%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdactor/test/actors_workers.cpp.o
[100%] Building CXX object test/CMakeFiles/run_all_tests.dir/__/tdactor/test/actors_bugs.cpp.o
[100%] Linking CXX executable run_all_tests
ld: warning: ignoring duplicate libraries: '../libtdcore.a'
[100%] Built target run_all_tests
Install the project...
-- Install configuration: "Release"
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdjson.1.8.66.dylib
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdjson.dylib
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdjson_static.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdjson_private.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdcore.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdmtproto.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdclient.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdapi.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdutils.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdactor.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tde2e.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdnet.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdsqlite.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tddb.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdmtproto.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdcore.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdclient.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdapi.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdjson_private.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdjson_static.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/pkgconfig/tdjson.pc
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/cmake/Td/TdTargets.cmake
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/cmake/Td/TdTargets-release.cmake
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/cmake/Td/TdStaticTargets.cmake
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/cmake/Td/TdStaticTargets-release.cmake
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_json_client.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_log.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/tdjson_export.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/Client.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/Log.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/tl/TlObject.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.hpp
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/cmake/Td/TdConfig.cmake
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/cmake/Td/TdConfigVersion.cmake
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdutils.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtde2e.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdactor.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdnet.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtdsqlite.a
-- Installing: /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/lib/libtddb.a
-- The CXX compiler identification is AppleClang 21.0.0.21000101
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.3s)
-- Generating done (0.0s)
CMake Warning (unused-cli):
  Manually-specified variables were not used by the project:

    CMAKE_EXPORT_NO_PACKAGE_REGISTRY


-- Build files have been written to: /Users/lizhao/Documents/Airport-Config/tg-video-cli/build
[ 50%] Building CXX object CMakeFiles/tg-video-cli.dir/src/main.cpp.o
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:758:24: error: no type named
      'authorizationStateWaitEncryptionKey' in namespace 'td::td_api'; did you mean 'authorizationStateWaitEmailCode'?
  758 |         [this](td_api::authorizationStateWaitEncryptionKey&) {
      |                ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      |                        authorizationStateWaitEmailCode
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h:1163:7: note: 
      'authorizationStateWaitEmailCode' declared here
 1163 | class authorizationStateWaitEmailCode final : public AuthorizationState {
      |       ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:759:44: error: no member named
      'checkDatabaseEncryptionKey' in namespace 'td::td_api'
  759 |           send(td_api::make_object<td_api::checkDatabaseEncryptionKey>(config_.database_encryption_key));
      |                                            ^~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:859:33: error: call to
      'parse_string_option' is ambiguous
  859 |   const std::string json_path = parse_string_option(args, "json", "");
      |                                 ^~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:48:13: note: candidate function
   48 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:812:13: note: candidate function
  812 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1000:37: error: call to
      'parse_string_option' is ambiguous
 1000 |   const std::filesystem::path out = parse_string_option(args, "out", "downloads");
      |                                     ^~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:48:13: note: candidate function
   48 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:812:13: note: candidate function
  812 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1001:32: error: call to
      'parse_string_option' is ambiguous
 1001 |   const std::string messages = parse_string_option(args, "messages", "");
      |                                ^~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:48:13: note: candidate function
   48 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:812:13: note: candidate function
  812 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1053:25: error: no viable
      overloaded '='
 1053 |   video_content->video_ = std::move(local_file);
      |   ~~~~~~~~~~~~~~~~~~~~~ ^ ~~~~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/tl/TlObject.h:101:15: note: 
      candidate function not viable: no known conversion from
      '__libcpp_remove_reference_t<td::tl::unique_ptr<td::td_api::inputFileLocal> &>' (aka
      'td::tl::unique_ptr<td::td_api::inputFileLocal>') to 'const unique_ptr<td::td_api::inputVideo>' for 1st argument
  101 |   unique_ptr &operator=(const unique_ptr &) = delete;
      |               ^         ~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/tl/TlObject.h:104:15: note: 
      candidate function not viable: no known conversion from
      '__libcpp_remove_reference_t<td::tl::unique_ptr<td::td_api::inputFileLocal> &>' (aka
      'td::tl::unique_ptr<td::td_api::inputFileLocal>') to 'unique_ptr<td::td_api::inputVideo>' for 1st argument
  104 |   unique_ptr &operator=(unique_ptr &&other) noexcept {
      |               ^         ~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/tl/TlObject.h:120:15: note: 
      candidate template ignored: requirement 'std::is_base_of<td::td_api::inputVideo,
      td::td_api::inputFileLocal>::value' was not satisfied [with S = td::td_api::inputFileLocal]
  120 |   unique_ptr &operator=(unique_ptr<S> &&other) noexcept {
      |               ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1054:18: error: no member named
      'supports_streaming_' in 'td::td_api::inputMessageVideo'
 1054 |   video_content->supports_streaming_ = true;
      |   ~~~~~~~~~~~~~~~^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1079:33: error: call to
      'parse_string_option' is ambiguous
 1079 |   const std::string json_path = parse_string_option(args, "json", "");
      |                                 ^~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:48:13: note: candidate function
   48 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:812:13: note: candidate function
  812 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1097:43: error: call to
      'parse_string_option' is ambiguous
 1097 |   const std::filesystem::path file_path = parse_string_option(args, "file", "");
      |                                           ^~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:48:13: note: candidate function
   48 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:812:13: note: candidate function
  812 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1098:31: error: call to
      'parse_string_option' is ambiguous
 1098 |   const std::string caption = parse_string_option(args, "caption", "");
      |                               ^~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:48:13: note: candidate function
   48 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);
      |             ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:812:13: note: candidate function
  812 | std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
      |             ^
In file included from /Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:1:
In file included from /Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/Client.h:11:
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h:34:31: error: 
      no matching constructor for initialization of 'td::td_api::addProxy'
   34 |   return object_ptr<Type>(new Type(std::forward<Args>(args)...));
      |                               ^    ~~~~~~~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/src/main.cpp:706:18: note: in instantiation of
      function template specialization 'td::td_api::make_object<td::td_api::addProxy,
      td::tl::unique_ptr<td::td_api::proxy>, bool>' requested here
  706 |     send(td_api::make_object<td_api::addProxy>(std::move(proxy), true));
      |                  ^
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h:41840:7: note: 
      candidate constructor (the implicit copy constructor) not viable: requires 1 argument, but 2 were provided
 41840 | class addProxy final : public Function {
       |       ^~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h:41840:7: note: 
      candidate constructor (the implicit move constructor) not viable: requires 1 argument, but 2 were provided
 41840 | class addProxy final : public Function {
       |       ^~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h:41852:3: note: 
      candidate constructor not viable: requires 3 arguments, but 2 were provided
 41852 |   addProxy(object_ptr<proxy> &&proxy_, bool enable_, string const &comment_);
       |   ^        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/lizhao/Documents/Airport-Config/tg-video-cli/vendor/tdlib/include/td/telegram/td_api.h:41850:3: note: 
      candidate constructor not viable: requires 0 arguments, but 2 were provided
 41850 |   addProxy();
       |   ^
11 errors generated.
make[2]: *** [CMakeFiles/tg-video-cli.dir/src/main.cpp.o] Error 1
make[1]: *** [CMakeFiles/tg-video-cli.dir/all] Error 2
make: *** [all] Error 2
```
