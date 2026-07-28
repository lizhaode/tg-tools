#ifndef TG_TOOLS_SRC_HELP_H_
#define TG_TOOLS_SRC_HELP_H_

#include <string>

namespace tg_tools {

bool IsKnownCommand(const std::string& command);
bool PrintHelp(const std::string& command = {});

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_HELP_H_
