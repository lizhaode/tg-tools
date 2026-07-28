#ifndef TG_TOOLS_SRC_ARGS_H_
#define TG_TOOLS_SRC_ARGS_H_

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace tg_tools {

struct ParsedArgs {
  std::map<std::string, std::string> options;
  std::vector<std::string> positional;

  bool HasFlag(const std::string& name) const;
};

ParsedArgs ParseArgs(int argc, char** argv);
bool ParseInt64Option(const ParsedArgs& args, const std::string& key,
                      std::int64_t* value, std::string* error);
bool ParseOptionalIntOption(const ParsedArgs& args, const std::string& key,
                            std::optional<int>* value, std::string* error);
std::string ParseStringOption(const ParsedArgs& args, const std::string& key,
                              const std::string& fallback);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_ARGS_H_
