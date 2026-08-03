#include "core/text_util.h"

#include <charconv>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

namespace tg_tools {
namespace {

bool IsUtf8ContinuationByte(unsigned char value) {
  return (value & 0xC0) == 0x80;
}

std::uint32_t DecodeNextUtf8CodePoint(const std::string& value,
                                      std::size_t* index) {
  const unsigned char first_byte = static_cast<unsigned char>(value[*index]);
  if (first_byte < 0x80) {
    ++(*index);
    return first_byte;
  }

  std::size_t byte_count = 0;
  std::uint32_t code_point = 0;
  std::uint32_t min_code_point = 0;
  if ((first_byte & 0xE0) == 0xC0) {
    byte_count = 2;
    code_point = first_byte & 0x1F;
    min_code_point = 0x80;
  } else if ((first_byte & 0xF0) == 0xE0) {
    byte_count = 3;
    code_point = first_byte & 0x0F;
    min_code_point = 0x800;
  } else if ((first_byte & 0xF8) == 0xF0) {
    byte_count = 4;
    code_point = first_byte & 0x07;
    min_code_point = 0x10000;
  } else {
    ++(*index);
    return first_byte;
  }

  if (*index + byte_count > value.size()) {
    ++(*index);
    return first_byte;
  }

  for (std::size_t offset = 1; offset < byte_count; ++offset) {
    const unsigned char next_byte =
        static_cast<unsigned char>(value[*index + offset]);
    if (!IsUtf8ContinuationByte(next_byte)) {
      ++(*index);
      return first_byte;
    }
    code_point = (code_point << 6) | (next_byte & 0x3F);
  }

  if (code_point < min_code_point ||
      (code_point >= 0xD800 && code_point <= 0xDFFF) || code_point > 0x10FFFF) {
    ++(*index);
    return first_byte;
  }

  *index += byte_count;
  return code_point;
}

bool IsZeroWidthCodePoint(std::uint32_t code_point) {
  return (code_point >= 0x0300 && code_point <= 0x036F) ||
         (code_point >= 0x1AB0 && code_point <= 0x1AFF) ||
         (code_point >= 0x1DC0 && code_point <= 0x1DFF) ||
         (code_point >= 0x20D0 && code_point <= 0x20FF) ||
         (code_point >= 0xFE00 && code_point <= 0xFE0F) ||
         (code_point >= 0xFE20 && code_point <= 0xFE2F) || code_point == 0x200D;
}

bool IsWideCodePoint(std::uint32_t code_point) {
  return code_point == 0x3000 ||
         (code_point >= 0x1100 && code_point <= 0x115F) ||
         (code_point >= 0x2329 && code_point <= 0x232A) ||
         (code_point >= 0x2E80 && code_point <= 0xA4CF) ||
         (code_point >= 0xAC00 && code_point <= 0xD7A3) ||
         (code_point >= 0xF900 && code_point <= 0xFAFF) ||
         (code_point >= 0xFE10 && code_point <= 0xFE19) ||
         (code_point >= 0xFE30 && code_point <= 0xFE6F) ||
         (code_point >= 0xFF00 && code_point <= 0xFF60) ||
         (code_point >= 0xFFE0 && code_point <= 0xFFE6) ||
         (code_point >= 0x1F000 && code_point <= 0x1FAFF) ||
         (code_point >= 0x2600 && code_point <= 0x27BF) ||
         (code_point >= 0x20000 && code_point <= 0x3FFFD);
}

std::size_t CodePointDisplayWidth(std::uint32_t code_point) {
  if (code_point == 0 || code_point < 0x20 ||
      (code_point >= 0x7F && code_point < 0xA0) ||
      IsZeroWidthCodePoint(code_point)) {
    return 0;
  }
  return IsWideCodePoint(code_point) ? 2 : 1;
}

}  // namespace

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
  std::string output;
  output.reserve(value.size());
  for (std::size_t index = 0; index < value.size(); ++index) {
    const unsigned char character = static_cast<unsigned char>(value[index]);
    if (character < 0x20 || character == 0x7F ||
        (character >= 0x80 && character <= 0x9F)) {
      output.push_back(' ');
      continue;
    }

    if (character == 0xC2 && index + 1 < value.size()) {
      const unsigned char next = static_cast<unsigned char>(value[index + 1]);
      if (next >= 0x80 && next <= 0x9F) {
        output.push_back(' ');
        ++index;
        continue;
      }
    }

    output.push_back(static_cast<char>(character));
  }
  return output;
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

std::size_t DisplayWidth(const std::string& value) {
  std::size_t width = 0;
  std::size_t index = 0;
  while (index < value.size()) {
    width += CodePointDisplayWidth(DecodeNextUtf8CodePoint(value, &index));
  }
  return width;
}

std::string PadRight(const std::string& value, std::size_t width) {
  const std::size_t current_width = DisplayWidth(value);
  if (current_width >= width) {
    return value;
  }
  return value + std::string(width - current_width, ' ');
}

std::string ClipDisplay(std::string value, std::size_t max_width) {
  value = OneLine(std::move(value));
  if (DisplayWidth(value) <= max_width) {
    return value;
  }

  const std::string marker = "....";
  const std::size_t marker_width = DisplayWidth(marker);
  if (max_width <= marker_width) {
    return marker.substr(0, max_width);
  }

  const std::size_t target_width = max_width - marker_width;
  std::string output;
  std::size_t output_width = 0;
  std::size_t index = 0;
  while (index < value.size()) {
    const std::size_t character_begin = index;
    const std::uint32_t code_point = DecodeNextUtf8CodePoint(value, &index);
    const std::size_t code_point_width = CodePointDisplayWidth(code_point);
    if (output_width + code_point_width > target_width) {
      break;
    }
    output.append(value, character_begin, index - character_begin);
    output_width += code_point_width;
  }
  return output + marker;
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
