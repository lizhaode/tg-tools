#include "core/text_util.h"

#include <charconv>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

namespace tg_tools {

bool SetError(std::string* error, std::string message) {
  if (error != nullptr) {
    *error = std::move(message);
  }
  return false;
}

bool ParseIntText(const std::string& text, int* value) {
  const char* begin = text.data();
  const char* end = text.data() + text.size();
  const auto result = std::from_chars(begin, end, *value);
  return result.ec == std::errc() && result.ptr == end;
}

bool ParseInt64Text(const std::string& text, std::int64_t* value) {
  const char* begin = text.data();
  const char* end = text.data() + text.size();
  const auto result = std::from_chars(begin, end, *value);
  return result.ec == std::errc() && result.ptr == end;
}

std::string Trim(const std::string& value) {
  const auto begin = value.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(begin, end - begin + 1);
}

std::string PromptLine(const std::string& prompt, const std::string& fallback) {
  if (!fallback.empty()) {
    return fallback;
  }
  std::cout << prompt;
  std::string value;
  std::getline(std::cin, value);
  return value;
}

std::string FormatTimestamp(std::int64_t timestamp) {
  if (timestamp <= 0) {
    return "-";
  }
  std::time_t time_value = static_cast<std::time_t>(timestamp);
  std::tm local_time{};
  localtime_r(&time_value, &local_time);
  char buffer[32]{};
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &local_time);
  return buffer;
}

std::string OneLine(std::string value) {
  for (char& character : value) {
    if (character == '\n' || character == '\r' || character == '\t') {
      character = ' ';
    }
  }
  return value;
}

std::string Clip(std::string value, std::size_t max_size) {
  value = OneLine(std::move(value));
  if (value.size() <= max_size) {
    return value;
  }
  const std::string marker = "....";
  if (max_size <= marker.size()) {
    return value.substr(0, max_size);
  }
  return value.substr(0, max_size - marker.size()) + marker;
}

std::string JsonEscape(const std::string& value) {
  std::ostringstream output;
  for (const unsigned char character : value) {
    switch (character) {
      case '"':
        output << "\\\"";
        break;
      case '\\':
        output << "\\\\";
        break;
      case '\b':
        output << "\\b";
        break;
      case '\f':
        output << "\\f";
        break;
      case '\n':
        output << "\\n";
        break;
      case '\r':
        output << "\\r";
        break;
      case '\t':
        output << "\\t";
        break;
      default:
        if (character < 0x20) {
          output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                 << static_cast<int>(character) << std::dec
                 << std::setfill(' ');
        } else {
          output << character;
        }
        break;
    }
  }
  return output.str();
}

}  // namespace tg_tools
