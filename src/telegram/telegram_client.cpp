#include "telegram/telegram_client.h"

#include <iostream>
#include <optional>
#include <utility>

#include <td/telegram/td_api.hpp>

#include "core/text_util.h"
#include "telegram/proxy.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

template <class... Fs>
struct Overloaded : Fs... {
  explicit Overloaded(Fs... fs) : Fs(std::move(fs))... {}
  using Fs::operator()...;
};

template <class... Fs>
auto MakeOverloaded(Fs... fs) {
  return Overloaded<Fs...>(std::move(fs)...);
}

}  // namespace

TelegramClient::TelegramClient(Config config) : config_(std::move(config)) {
  td::ClientManager::execute(
      td_api::make_object<td_api::setLogVerbosityLevel>(1));
  client_manager_ = std::make_unique<td::ClientManager>();
  client_id_ = client_manager_->create_client_id();
}

TelegramClient::~TelegramClient() {
  if (client_manager_) {
    Send(td_api::make_object<td_api::close>());
  }
}

bool TelegramClient::EnsureAuthorized(std::string* error) {
  if (!ValidateConfig(config_, error)) {
    return false;
  }
  if (!ConfigureProxyFromEnvironment(error)) {
    return false;
  }
  Send(td_api::make_object<td_api::getOption>("version"));
  while (!is_authorized_) {
    if (!ProcessResponse(client_manager_->receive(10), error)) {
      return false;
    }
  }
  return true;
}

TelegramClient::Object TelegramClient::Request(Function function,
                                               std::chrono::seconds timeout,
                                               std::string* error,
                                               bool allow_error) {
  const auto request_id = next_query_id_++;
  client_manager_->send(client_id_, request_id, std::move(function));

  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (std::chrono::steady_clock::now() < deadline) {
    auto response = client_manager_->receive(1.0);
    if (!response.object) {
      continue;
    }
    if (response.request_id == 0) {
      if (!ProcessUpdate(std::move(response.object), error)) {
        return {};
      }
      continue;
    }
    if (response.request_id == request_id) {
      if (!allow_error && !CheckTdError(*response.object, error)) {
        return {};
      }
      return std::move(response.object);
    }
    if (!CheckTdError(*response.object, error)) {
      return {};
    }
  }
  SetError(error, "等待 TDLib 响应超时");
  return {};
}

td::ClientManager::Response TelegramClient::Receive(double timeout_seconds) {
  return client_manager_->receive(timeout_seconds);
}

void TelegramClient::Send(Function function) {
  client_manager_->send(client_id_, next_query_id_++, std::move(function));
}

bool TelegramClient::ConfigureProxyFromEnvironment(std::string* error) {
  std::optional<ProxyConfig> proxy_config;
  if (!ProxyFromEnvironment(&proxy_config, error)) {
    return false;
  }
  if (!proxy_config) {
    return true;
  }

  auto proxy_type = td_api::make_object<td_api::proxyTypeHttp>("", "", false);
  auto proxy = td_api::make_object<td_api::proxy>(
      proxy_config->server, proxy_config->port, std::move(proxy_type));
  Send(td_api::make_object<td_api::addProxy>(std::move(proxy), true, ""));
  std::cout << "已启用代理：http://" << proxy_config->server << ':'
            << proxy_config->port << '\n';
  return true;
}

bool TelegramClient::CheckTdError(const td_api::Object& object,
                                  std::string* error) {
  if (object.get_id() == td_api::error::ID) {
    const auto& td_error = static_cast<const td_api::error&>(object);
    return SetError(error, "TDLib error " + std::to_string(td_error.code_) +
                               ": " + td_error.message_);
  }
  return true;
}

bool TelegramClient::ProcessResponse(td::ClientManager::Response response,
                                     std::string* error) {
  if (!response.object) {
    return true;
  }
  if (response.request_id == 0) {
    return ProcessUpdate(std::move(response.object), error);
  }
  return CheckTdError(*response.object, error);
}

bool TelegramClient::ProcessUpdate(Object update, std::string* error) {
  bool ok = true;
  td_api::downcast_call(
      *update,
      MakeOverloaded(
          [this, &ok, error](td_api::updateAuthorizationState& update) {
            authorization_state_ = std::move(update.authorization_state_);
            ok = OnAuthorizationStateUpdate(error);
          },
          [](auto&) {}));
  return ok;
}

bool TelegramClient::OnAuthorizationStateUpdate(std::string* error) {
  bool ok = true;
  td_api::downcast_call(
      *authorization_state_,
      MakeOverloaded(
          [this](td_api::authorizationStateReady&) { is_authorized_ = true; },
          [this](td_api::authorizationStateWaitTdlibParameters&) {
            auto request = td_api::make_object<td_api::setTdlibParameters>();
            request->use_test_dc_ = false;
            request->database_directory_ = config_.database_directory;
            request->files_directory_ = config_.files_directory;
            request->database_encryption_key_ = config_.database_encryption_key;
            request->use_file_database_ = true;
            request->use_chat_info_database_ = true;
            request->use_message_database_ = true;
            request->use_secret_chats_ = false;
            request->api_id_ = config_.api_id;
            request->api_hash_ = config_.api_hash;
            request->system_language_code_ = config_.system_language_code;
            request->device_model_ = config_.device_model;
            request->system_version_ = config_.system_version;
            request->application_version_ = config_.application_version;
            Send(std::move(request));
          },
          [this](td_api::authorizationStateWaitPhoneNumber&) {
            const std::string phone_number =
                PromptLine("手机号（含国家区号，例如 +8613800000000）：",
                           config_.phone_number);
            Send(td_api::make_object<td_api::setAuthenticationPhoneNumber>(
                phone_number, nullptr));
          },
          [this](td_api::authorizationStateWaitCode&) {
            const std::string code = PromptLine("登录验证码：");
            Send(td_api::make_object<td_api::checkAuthenticationCode>(code));
          },
          [this](td_api::authorizationStateWaitPassword&) {
            const std::string password =
                PromptLine("两步验证密码（输入可见）：");
            Send(td_api::make_object<td_api::checkAuthenticationPassword>(
                password));
          },
          [](td_api::authorizationStateWaitOtherDeviceConfirmation& state) {
            std::cout << "请在另一个已登录的 Telegram 客户端确认登录："
                      << state.link_ << '\n';
          },
          [&](td_api::authorizationStateWaitRegistration&) {
            ok = SetError(
                error,
                "当前工具不支持注册新账号，请使用已有 Telegram 账号登录");
          },
          [&](td_api::authorizationStateClosed&) {
            ok = SetError(error, "TDLib 登录流程已关闭");
          },
          [](auto&) {}));
  return ok;
}

}  // namespace tg_tools
