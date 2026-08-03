#ifndef TG_TOOLS_SRC_TELEGRAM_TELEGRAM_CLIENT_H_
#define TG_TOOLS_SRC_TELEGRAM_TELEGRAM_CLIENT_H_

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>

#include "core/config.h"

namespace tg_tools {

class TelegramClient {
 public:
  using Object = td::td_api::object_ptr<td::td_api::Object>;
  using Function = td::td_api::object_ptr<td::td_api::Function>;

  explicit TelegramClient(Config config);
  ~TelegramClient();

  bool EnsureAuthorized(std::string* error);
  Object Request(Function function, std::chrono::seconds timeout,
                 std::string* error, bool allow_error = false);
  td::ClientManager::Response Receive(double timeout_seconds);
  std::uint64_t Send(Function function);

 private:
  bool ConfigureProxyFromEnvironment(std::string* error);
  static bool CheckTdError(const td::td_api::Object& object,
                           std::string* error);
  bool ProcessResponse(td::ClientManager::Response response,
                       std::string* error);
  bool ProcessUpdate(Object update, std::string* error);
  bool OnAuthorizationStateUpdate(std::string* error);

  Config config_;
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_ = 0;
  std::uint64_t next_query_id_ = 1;
  bool is_authorized_ = false;
  td::td_api::object_ptr<td::td_api::AuthorizationState> authorization_state_;
};

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_TELEGRAM_TELEGRAM_CLIENT_H_
