#ifndef TG_TOOLS_SRC_CORE_TEXT_UTIL_H_
#define TG_TOOLS_SRC_CORE_TEXT_UTIL_H_

#include <cstddef>
#include <cstdint>
#include <string>

namespace tg_tools {

bool SetError(std::string* error, std::string message);
bool ParseIntText(const std::string& text, int* value);
bool ParseInt64Text(const std::string& text, std::int64_t* value);
std::string Trim(const std::string& value);
std::string PromptLine(const std::string& prompt,
                       const std::string& fallback = {});
std::string FormatTimestamp(std::int64_t timestamp);
std::string FormatSize(std::int64_t size);
std::string OneLine(std::string value);
std::string Clip(std::string value, std::size_t max_size);
std::size_t DisplayWidth(const std::string& value);
std::string PadRight(const std::string& value, std::size_t width);
std::string PadLeft(const std::string& value, std::size_t width);
std::string ClipDisplay(std::string value, std::size_t max_width);
std::string JsonEscape(const std::string& value);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_CORE_TEXT_UTIL_H_
