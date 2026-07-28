#include "core/args.h"

#include <charconv>
#include <system_error>
#include <utility>

namespace tg_tools {
namespace {

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

}  // namespace

ParsedArgs ParseArgs(int argc, char** argv) {
  ParsedArgs args;
  for (int arg_index = 1; arg_index < argc; ++arg_index) {
    std::string token = argv[arg_index];
    if (token.rfind("--", 0) == 0) {
      token = token.substr(2);
      const std::size_t equals_position = token.find('=');
      if (equals_position != std::string::npos) {
        args.options[token.substr(0, equals_position)] =
            token.substr(equals_position + 1);
      } else if (arg_index + 1 < argc &&
                 std::string(argv[arg_index + 1]).rfind("--", 0) != 0) {
        args.options[token] = argv[++arg_index];
      } else {
        args.options[token] = "true";
      }
    } else {
      args.positional.push_back(token);
    }
  }
  return args;
}

bool ParsedArgs::HasFlag(const std::string& name) const {
  const auto iterator = options.find(name);
  return iterator != options.end() && iterator->second == "true";
}

bool ParseInt64Option(const ParsedArgs& args, const std::string& key,
                      std::int64_t* value, std::string* error) {
  const auto iterator = args.options.find(key);
  if (iterator == args.options.end() || iterator->second.empty()) {
    return SetError(error, "缺少必填参数 --" + key);
  }
  if (value == nullptr || !ParseInt64Text(iterator->second, value)) {
    return SetError(error, "参数 --" + key + " 必须是整数");
  }
  return true;
}

bool ParseOptionalIntOption(const ParsedArgs& args, const std::string& key,
                            std::optional<int>* value, std::string* error) {
  const auto iterator = args.options.find(key);
  if (value == nullptr) {
    return SetError(error, "内部错误：参数输出指针为空");
  }
  if (iterator == args.options.end() || iterator->second.empty()) {
    value->reset();
    return true;
  }

  int parsed_value = 0;
  if (!ParseIntText(iterator->second, &parsed_value)) {
    return SetError(error, "参数 --" + key + " 必须是整数");
  }
  *value = parsed_value;
  return true;
}

std::string ParseStringOption(const ParsedArgs& args, const std::string& key,
                              const std::string& fallback) {
  const auto iterator = args.options.find(key);
  return iterator == args.options.end() ? fallback : iterator->second;
}

}  // namespace tg_tools
