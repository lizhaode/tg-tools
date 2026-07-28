#include "telegram/proxy.h"

#include <cstdlib>

#include "core/text_util.h"

namespace tg_tools {
namespace {

std::string EnvOrEmpty(const char* name) {
  const char* value = std::getenv(name);
  return value == nullptr ? std::string() : std::string(value);
}

}  // namespace

bool ParseProxyUrl(std::string proxy_url, ProxyConfig* config,
                   std::string* error) {
  proxy_url = Trim(proxy_url);
  if (config == nullptr) {
    return SetError(error, "内部错误：代理配置输出指针为空");
  }

  const std::string prefix = "http://";
  if (proxy_url.rfind(prefix, 0) != 0) {
    return SetError(error, "代理地址格式必须是 http://host:port");
  }

  std::string authority = proxy_url.substr(prefix.size());

  const auto path_separator = authority.find('/');
  if (path_separator != std::string::npos) {
    authority = authority.substr(0, path_separator);
  }

  const auto port_separator = authority.rfind(':');
  if (port_separator == std::string::npos) {
    return SetError(error, "代理地址必须包含端口，例如 http://127.0.0.1:7890");
  }

  int port = 0;
  const std::string port_text = authority.substr(port_separator + 1);
  if (!ParseIntText(port_text, &port)) {
    return SetError(error, "代理地址端口必须是整数");
  }
  config->server = authority.substr(0, port_separator);
  config->port = port;
  if (config->server.empty() || config->port <= 0 || config->port > 65535) {
    return SetError(error, "代理地址无效：host 为空或端口不合法");
  }

  return true;
}

bool ProxyFromEnvironment(std::optional<ProxyConfig>* proxy_config,
                          std::string* error) {
  if (proxy_config == nullptr) {
    return SetError(error, "内部错误：代理配置输出指针为空");
  }
  proxy_config->reset();
  const char* names[] = {"ALL_PROXY",   "all_proxy",  "HTTPS_PROXY",
                         "https_proxy", "HTTP_PROXY", "http_proxy"};
  for (const char* name : names) {
    const std::string value = EnvOrEmpty(name);
    if (!Trim(value).empty()) {
      ProxyConfig parsed_config;
      if (!ParseProxyUrl(value, &parsed_config, error)) {
        return false;
      }
      *proxy_config = parsed_config;
      return true;
    }
  }
  return true;
}

}  // namespace tg_tools
