cmake_minimum_required(VERSION 3.24)

project(tg-tools LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(tg-tools
  src/cli.cpp
  src/commands/chats.cpp
  src/commands/download.cpp
  src/commands/messages.cpp
  src/commands/upload.cpp
  src/commands/upload_json.cpp
  src/core/args.cpp
  src/core/config.cpp
  src/core/text_util.cpp
  src/help.cpp
  src/main.cpp
  src/telegram/message_format.cpp
  src/telegram/proxy.cpp
  src/telegram/telegram_client.cpp
)

target_compile_features(tg-tools PRIVATE cxx_std_17)
target_include_directories(tg-tools PRIVATE src)
set_target_properties(tg-tools PROPERTIES CXX_EXTENSIONS OFF)

find_package(Td REQUIRED)
target_link_libraries(tg-tools PRIVATE Td::TdStatic nlohmann_json::nlohmann_json)

target_compile_options(tg-tools PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -fno-exceptions
  "$<$<CONFIG:Release>:-ffunction-sections>"
  "$<$<CONFIG:Release>:-fdata-sections>"
)
target_link_options(tg-tools PRIVATE
  "$<$<CONFIG:Release>:-Wl,-dead_strip>"
)
