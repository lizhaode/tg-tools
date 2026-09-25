cmake_minimum_required(VERSION 3.24)

project(tg-tools LANGUAGES CXX)

add_executable(tg-tools
  src/cli.cpp
  src/commands/chats.cpp
  src/commands/download.cpp
  src/commands/messages.cpp
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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tdlib")
  # scripts/build.sh 把 TDLib 安装到 vendor/tdlib，让 CLion 等直接配置也能找到
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tdlib")
endif()
find_package(Td REQUIRED)
target_link_libraries(tg-tools PRIVATE Td::TdStatic)

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
