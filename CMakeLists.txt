cmake_minimum_required(VERSION 3.16)

project(tg-video-cli VERSION 0.1.0 LANGUAGES CXX)

add_executable(tg-video-cli
  src/main.cpp
)

target_compile_features(tg-video-cli PRIVATE cxx_std_17)

find_package(Td REQUIRED)
target_link_libraries(tg-video-cli PRIVATE Td::TdStatic)

target_compile_options(tg-video-cli PRIVATE -Wall -Wextra -Wpedantic)
