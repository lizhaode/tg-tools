#ifndef TG_TOOLS_SRC_PROXY_H_
#define TG_TOOLS_SRC_PROXY_H_

#include <optional>
#include <string>

namespace tg_tools {

struct ProxyConfig {
  std::string server;
  int port = 0;
};

bool ParseProxyUrl(std::string proxy_url, ProxyConfig* config,
                   std::string* error);
bool ProxyFromEnvironment(std::optional<ProxyConfig>* proxy_config,
                          std::string* error);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_PROXY_H_
