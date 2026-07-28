cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
set(CMAKE_CONFIGURATION_TYPES Release CACHE STRING "Build configurations" FORCE)
set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "Build architecture" FORCE)

project(tg-tools VERSION 0.1.0 LANGUAGES CXX)

if(NOT APPLE OR NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  message(FATAL_ERROR "tg-tools supports Apple Silicon macOS arm64 only")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
target_link_libraries(tg-tools PRIVATE Td::TdStatic)

target_compile_options(tg-tools PRIVATE -Wall -Wextra -Wpedantic -fno-exceptions)
target_link_options(tg-tools PRIVATE -Wl,-dead_strip)
