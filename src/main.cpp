#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tg_video_cli {

struct Config {
  int api_id = 0;
  std::string api_hash;
  std::string phone_number;
  std::string database_directory = "tdlib-db";
  std::string files_directory = "tdlib-files";
  std::string database_encryption_key = "tgvideoclikey";
  std::string system_language_code = "zh-CN";
  std::string device_model = "CLI";
  std::string system_version;
  std::string application_version = "0.1.0";
};

struct ParsedArgs {
  std::map<std::string, std::string> options;
  std::vector<std::string> positional;

  bool has_flag(const std::string& name) const;
};

std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback);

namespace {

namespace td_api = td::td_api;
using Object = td_api::object_ptr<td_api::Object>;
using Function = td_api::object_ptr<td_api::Function>;

template <class... Fs>
struct Overloaded : Fs... {
  explicit Overloaded(Fs... fs) : Fs(std::move(fs))... {}
  using Fs::operator()...;
};

template <class... Fs>
auto overloaded(Fs... fs) {
  return Overloaded<Fs...>(std::move(fs)...);
}

struct VideoFile {
  int file_id = 0;
  std::string file_name;
  std::string mime_type;
};

struct PendingDownload {
  std::int64_t message_id = 0;
  std::filesystem::path destination;
};

struct DownloadFailure {
  std::int64_t message_id = 0;
  std::int32_t file_id = 0;
  std::filesystem::path destination;
  std::string reason;
};

struct UploadItem {
  std::filesystem::path name;
  std::string caption;
};

struct UploadFailure {
  std::filesystem::path name;
  std::string caption;
  std::string reason;
};

constexpr int kDownloadParallel = 3;

struct ProxyConfig {
  std::string server;
  int port = 0;
};

std::string env_or_empty(const char* name) {
  const char* value = std::getenv(name);
  return value == nullptr ? std::string() : std::string(value);
}

std::string trim(std::string value) {
  const auto begin = value.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(begin, end - begin + 1);
}

ProxyConfig parse_proxy_url(std::string proxy_url) {
  proxy_url = trim(std::move(proxy_url));
  ProxyConfig config;

  const std::string prefix = "http://";
  if (proxy_url.rfind(prefix, 0) != 0) {
    throw std::runtime_error("proxy URL must use format http://host:port");
  }

  std::string authority = proxy_url.substr(prefix.size());

  const auto path_separator = authority.find('/');
  if (path_separator != std::string::npos) {
    authority = authority.substr(0, path_separator);
  }

  const auto port_separator = authority.rfind(':');
  if (port_separator == std::string::npos) {
    throw std::runtime_error("proxy URL must include a port, for example http://127.0.0.1:78980");
  }

  config.server = authority.substr(0, port_separator);
  config.port = std::stoi(authority.substr(port_separator + 1));
  if (config.server.empty() || config.port <= 0 || config.port > 65535) {
    throw std::runtime_error("invalid proxy URL: host or port is invalid");
  }

  return config;
}

std::optional<ProxyConfig> proxy_from_environment() {
  const char* names[] = {"ALL_PROXY", "all_proxy", "HTTPS_PROXY", "https_proxy", "HTTP_PROXY", "http_proxy"};
  for (const char* name : names) {
    const std::string value = env_or_empty(name);
    if (!trim(value).empty()) {
      return parse_proxy_url(value);
    }
  }
  return std::nullopt;
}

std::map<std::string, std::string> read_key_value_file(const std::filesystem::path& path) {
  std::map<std::string, std::string> values;
  if (!std::filesystem::exists(path)) {
    return values;
  }

  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("failed to open " + path.string());
  }

  std::string line;
  while (std::getline(input, line)) {
    line = trim(line);
    if (line.empty() || line.front() == '#') {
      continue;
    }
    const auto separator = line.find('=');
    if (separator == std::string::npos) {
      continue;
    }
    values[trim(line.substr(0, separator))] = trim(line.substr(separator + 1));
  }
  return values;
}

std::string map_string_or(const std::map<std::string, std::string>& values, const std::string& key, std::string fallback) {
  const auto iterator = values.find(key);
  return iterator == values.end() ? std::move(fallback) : iterator->second;
}

int map_int_or(const std::map<std::string, std::string>& values, const std::string& key, int fallback) {
  const auto iterator = values.find(key);
  if (iterator == values.end() || iterator->second.empty()) {
    return fallback;
  }
  return std::stoi(iterator->second);
}

Config load_config(const std::filesystem::path& path) {
  const auto values = read_key_value_file(path);
  Config config;
  config.api_id = map_int_or(values, "api_id", config.api_id);
  config.api_hash = map_string_or(values, "api_hash", config.api_hash);
  config.phone_number = map_string_or(values, "phone_number", config.phone_number);
  config.database_directory = map_string_or(values, "database_directory", config.database_directory);
  config.files_directory = map_string_or(values, "files_directory", config.files_directory);
  config.database_encryption_key = map_string_or(values, "database_encryption_key", config.database_encryption_key);
  config.system_language_code = map_string_or(values, "system_language_code", config.system_language_code);
  config.device_model = map_string_or(values, "device_model", config.device_model);
  config.system_version = map_string_or(values, "system_version", config.system_version);
  config.application_version = map_string_or(values, "application_version", config.application_version);
  return config;
}

void validate_config(const Config& config) {
  if (config.api_id <= 0 || config.api_hash.empty() || config.api_hash == "PUT_YOUR_API_HASH_HERE") {
    throw std::runtime_error("missing Telegram api_id/api_hash. Copy config/telegram.example.conf to config/telegram.conf and edit it.");
  }
}

std::string prompt_line(const std::string& prompt, const std::string& fallback = {}) {
  if (!fallback.empty()) {
    return fallback;
  }
  std::cout << prompt;
  std::string value;
  std::getline(std::cin, value);
  return value;
}

std::string format_timestamp(std::int64_t timestamp) {
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

std::string chat_type_label(const td_api::chat& chat) {
  switch (chat.type_->get_id()) {
    case td_api::chatTypeBasicGroup::ID:
      return "basic_group";
    case td_api::chatTypeSupergroup::ID: {
      const auto& supergroup = static_cast<const td_api::chatTypeSupergroup&>(*chat.type_);
      return supergroup.is_channel_ ? "channel" : "supergroup";
    }
    case td_api::chatTypePrivate::ID:
      return "private";
    case td_api::chatTypeSecret::ID:
      return "secret";
    default:
      return "unknown";
  }
}

std::optional<VideoFile> extract_video_file(const td_api::message& message) {
  switch (message.content_->get_id()) {
    case td_api::messageVideo::ID: {
      const auto& content = static_cast<const td_api::messageVideo&>(*message.content_);
      return VideoFile{content.video_->video_->id_, content.video_->file_name_, content.video_->mime_type_};
    }
    case td_api::messageVideoNote::ID: {
      const auto& content = static_cast<const td_api::messageVideoNote&>(*message.content_);
      return VideoFile{content.video_note_->video_->id_, "", "video/mp4"};
    }
    case td_api::messageDocument::ID: {
      const auto& content = static_cast<const td_api::messageDocument&>(*message.content_);
      if (content.document_->mime_type_.rfind("video/", 0) == 0) {
        return VideoFile{content.document_->document_->id_, content.document_->file_name_, content.document_->mime_type_};
      }
      return std::nullopt;
    }
    default:
      return std::nullopt;
  }
}

std::string one_line(std::string value) {
  for (char& character : value) {
    if (character == '\n' || character == '\r' || character == '\t') {
      character = ' ';
    }
  }
  return value;
}

std::string clip(std::string value, std::size_t max_size) {
  value = one_line(std::move(value));
  if (value.size() <= max_size) {
    return value;
  }
  const std::string marker = "....";
  if (max_size <= marker.size()) {
    return value.substr(0, max_size);
  }
  return value.substr(0, max_size - marker.size()) + marker;
}

std::string json_escape(const std::string& value) {
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
          output << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(character) << std::dec << std::setfill(' ');
        } else {
          output << character;
        }
        break;
    }
  }
  return output.str();
}

struct MessageRow {
  std::string type;
  std::string file_name;
  std::string text;
  std::string mime_type;
  std::int32_t file_id = 0;
  std::int32_t duration = 0;
  std::int32_t width = 0;
  std::int32_t height = 0;
};

MessageRow describe_message(const td_api::message& message) {
  MessageRow row;

  switch (message.content_->get_id()) {
    case td_api::messageText::ID: {
      const auto& content = static_cast<const td_api::messageText&>(*message.content_);
      row.type = "text";
      row.text = content.text_->text_;
      break;
    }
    case td_api::messagePhoto::ID: {
      const auto& content = static_cast<const td_api::messagePhoto&>(*message.content_);
      row.type = "photo";
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    case td_api::messageVideo::ID: {
      const auto& content = static_cast<const td_api::messageVideo&>(*message.content_);
      row.type = "video";
      row.file_id = content.video_->video_->id_;
      row.file_name = content.video_->file_name_;
      row.mime_type = content.video_->mime_type_;
      row.duration = content.video_->duration_;
      row.width = content.video_->width_;
      row.height = content.video_->height_;
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    case td_api::messageVideoNote::ID: {
      const auto& content = static_cast<const td_api::messageVideoNote&>(*message.content_);
      row.type = "video_note";
      row.file_id = content.video_note_->video_->id_;
      row.mime_type = "video/mp4";
      row.duration = content.video_note_->duration_;
      row.width = content.video_note_->length_;
      row.height = content.video_note_->length_;
      break;
    }
    case td_api::messageDocument::ID: {
      const auto& content = static_cast<const td_api::messageDocument&>(*message.content_);
      row.type = "document";
      row.file_id = content.document_->document_->id_;
      row.file_name = content.document_->file_name_;
      row.mime_type = content.document_->mime_type_;
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    case td_api::messageAnimation::ID: {
      const auto& content = static_cast<const td_api::messageAnimation&>(*message.content_);
      row.type = "animation";
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    default:
      row.type = "type_" + std::to_string(message.content_->get_id());
      break;
  }

  return row;
}

void print_message_header() {
  std::cout << std::left << std::setw(14) << "message_id" << std::setw(17) << "date" << std::setw(10) << "type"
            << std::setw(124) << "file" << "text" << '\n';
}

void print_message_row(const td_api::message& message) {
  const MessageRow row = describe_message(message);
  std::cout << std::left << std::setw(14) << message.id_ << std::setw(17) << format_timestamp(message.date_)
            << std::setw(10) << row.type << std::setw(124) << clip(row.file_name, 120) << clip(row.text, 120) << '\n';
}

void write_message_json(std::ostream& output, const td_api::message& message) {
  const MessageRow row = describe_message(message);
  output << "  {";
  output << "\"message_id\":" << message.id_;
  output << ",\"date\":" << message.date_;
  output << ",\"date_text\":\"" << json_escape(format_timestamp(message.date_)) << "\"";
  output << ",\"type\":\"" << json_escape(row.type) << "\"";
  output << ",\"file_id\":" << row.file_id;
  output << ",\"file_name\":\"" << json_escape(row.file_name) << "\"";
  output << ",\"mime_type\":\"" << json_escape(row.mime_type) << "\"";
  output << ",\"duration\":" << row.duration;
  output << ",\"width\":" << row.width;
  output << ",\"height\":" << row.height;
  output << ",\"text\":\"" << json_escape(row.text) << "\"";
  output << "}";
}

std::string default_extension(const std::string& mime_type) {
  if (mime_type == "video/quicktime") {
    return ".mov";
  }
  if (mime_type == "video/x-matroska") {
    return ".mkv";
  }
  return ".mp4";
}

std::string safe_file_name(const VideoFile& video, std::int64_t message_id) {
  std::filesystem::path candidate(video.file_name);
  std::string file_name = candidate.filename().string();
  if (file_name.empty()) {
    file_name = "telegram_video_" + std::to_string(message_id) + default_extension(video.mime_type);
  }
  return file_name;
}

void save_downloaded_file(const std::filesystem::path& source, const std::filesystem::path& destination) {
  if (!destination.parent_path().empty()) {
    std::filesystem::create_directories(destination.parent_path());
  }
  std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
}

std::filesystem::path download_destination(const std::filesystem::path& out, const VideoFile& video, std::int64_t message_id) {
  if (out.has_extension()) {
    return out;
  }
  return out / safe_file_name(video, message_id);
}

std::vector<std::string> split_csv(const std::string& value) {
  std::vector<std::string> items;
  std::stringstream stream(value);
  std::string item;
  while (std::getline(stream, item, ',')) {
    item = trim(item);
    if (!item.empty()) {
      items.push_back(item);
    }
  }
  return items;
}

std::vector<std::int64_t> parse_message_ids(const std::string& value) {
  std::vector<std::int64_t> message_ids;
  for (const std::string& item : split_csv(value)) {
    message_ids.push_back(std::stoll(item));
  }
  if (message_ids.empty()) {
    throw std::runtime_error("--messages must contain at least one message id");
  }
  return message_ids;
}

std::string read_text_file(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("failed to open file: " + path.string());
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void skip_json_space(const std::string& text, std::size_t& index) {
  while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index]))) {
    ++index;
  }
}

std::string parse_json_string_value(const std::string& text, std::size_t& index) {
  skip_json_space(text, index);
  if (index >= text.size() || text[index] != '"') {
    throw std::runtime_error("expected JSON string");
  }
  ++index;

  std::string value;
  while (index < text.size()) {
    const char character = text[index++];
    if (character == '"') {
      return value;
    }
    if (character != '\\') {
      value.push_back(character);
      continue;
    }
    if (index >= text.size()) {
      throw std::runtime_error("invalid JSON escape");
    }
    const char escaped = text[index++];
    switch (escaped) {
      case '"':
        value.push_back('"');
        break;
      case '\\':
        value.push_back('\\');
        break;
      case 'n':
        value.push_back('\n');
        break;
      case 'r':
        value.push_back('\r');
        break;
      case 't':
        value.push_back('\t');
        break;
      default:
        value.push_back(escaped);
        break;
    }
  }
  throw std::runtime_error("unterminated JSON string");
}

UploadItem parse_upload_object(const std::string& text, std::size_t& index) {
  skip_json_space(text, index);
  if (index >= text.size() || text[index] != '{') {
    throw std::runtime_error("expected JSON object");
  }
  ++index;

  UploadItem item;
  while (true) {
    skip_json_space(text, index);
    if (index < text.size() && text[index] == '}') {
      ++index;
      break;
    }

    const std::string key = parse_json_string_value(text, index);
    skip_json_space(text, index);
    if (index >= text.size() || text[index] != ':') {
      throw std::runtime_error("expected ':' in upload JSON object");
    }
    ++index;
    const std::string value = parse_json_string_value(text, index);

    if (key == "name") {
      item.name = value;
    } else if (key == "caption") {
      item.caption = value;
    }

    skip_json_space(text, index);
    if (index < text.size() && text[index] == ',') {
      ++index;
      continue;
    }
    if (index < text.size() && text[index] == '}') {
      ++index;
      break;
    }
    throw std::runtime_error("expected ',' or '}' in upload JSON object");
  }

  if (item.name.empty()) {
    throw std::runtime_error("upload JSON object is missing name");
  }
  return item;
}

std::vector<UploadItem> parse_upload_json_file(const std::filesystem::path& path) {
  const std::string text = read_text_file(path);
  std::size_t index = 0;
  std::vector<UploadItem> items;
  skip_json_space(text, index);

  if (index >= text.size() || text[index] != '[') {
    throw std::runtime_error("upload JSON must be an array");
  }
  ++index;

  while (true) {
    skip_json_space(text, index);
    if (index < text.size() && text[index] == ']') {
      ++index;
      break;
    }
    items.push_back(parse_upload_object(text, index));
    skip_json_space(text, index);
    if (index < text.size() && text[index] == ',') {
      ++index;
    }
  }

  if (items.empty()) {
    throw std::runtime_error("upload JSON has no items");
  }
  return items;
}

class TelegramClient {
 public:
  explicit TelegramClient(Config config) : config_(std::move(config)) {
    td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
  }

  ~TelegramClient() {
    if (client_manager_) {
      send(td_api::make_object<td_api::close>());
    }
  }

  void ensure_authorized() {
    validate_config(config_);
    configure_proxy_from_environment();
    send(td_api::make_object<td_api::getOption>("version"));
    while (!is_authorized_) {
      process_response(client_manager_->receive(10));
    }
  }

  Object request(Function function, std::chrono::seconds timeout, bool allow_error = false) {
    const auto request_id = next_query_id_++;
    client_manager_->send(client_id_, request_id, std::move(function));

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
      auto response = client_manager_->receive(1.0);
      if (!response.object) {
        continue;
      }
      if (response.request_id == 0) {
        process_update(std::move(response.object));
        continue;
      }
      if (response.request_id == request_id) {
        if (!allow_error) {
          throw_if_error(*response.object);
        }
        return std::move(response.object);
      }
      throw_if_error(*response.object);
    }
    throw std::runtime_error("timed out waiting for TDLib response");
  }

  td::ClientManager::Response receive(double timeout_seconds) {
    return client_manager_->receive(timeout_seconds);
  }

  void send(Function function) {
    client_manager_->send(client_id_, next_query_id_++, std::move(function));
  }

 private:
  void configure_proxy_from_environment() {
    std::optional<ProxyConfig> proxy_config = proxy_from_environment();
    if (!proxy_config) {
      return;
    }

    auto proxy_type = td_api::make_object<td_api::proxyTypeHttp>("", "", false);
    auto proxy = td_api::make_object<td_api::proxy>(proxy_config->server, proxy_config->port, std::move(proxy_type));
    send(td_api::make_object<td_api::addProxy>(std::move(proxy), true));
    std::cout << "using Telegram proxy from environment: http://" << proxy_config->server << ':' << proxy_config->port << '\n';
  }

  static void throw_if_error(const td_api::Object& object) {
    if (object.get_id() == td_api::error::ID) {
      const auto& error = static_cast<const td_api::error&>(object);
      throw std::runtime_error("TDLib error " + std::to_string(error.code_) + ": " + error.message_);
    }
  }

  void process_response(td::ClientManager::Response response) {
    if (!response.object) {
      return;
    }
    if (response.request_id == 0) {
      process_update(std::move(response.object));
    } else {
      throw_if_error(*response.object);
    }
  }

  void process_update(Object update) {
    td_api::downcast_call(*update, overloaded(
        [this](td_api::updateAuthorizationState& authorization_update) {
          authorization_state_ = std::move(authorization_update.authorization_state_);
          on_authorization_state_update();
        },
        [](auto&) {}));
  }

  void on_authorization_state_update() {
    td_api::downcast_call(*authorization_state_, overloaded(
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
          send(std::move(request));
        },
        [this](td_api::authorizationStateWaitEncryptionKey&) {
          send(td_api::make_object<td_api::checkDatabaseEncryptionKey>(config_.database_encryption_key));
        },
        [this](td_api::authorizationStateWaitPhoneNumber&) {
          const std::string phone_number = prompt_line("phone number: ", config_.phone_number);
          send(td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone_number, nullptr));
        },
        [this](td_api::authorizationStateWaitCode&) {
          const std::string code = prompt_line("login code: ");
          send(td_api::make_object<td_api::checkAuthenticationCode>(code));
        },
        [this](td_api::authorizationStateWaitPassword&) {
          const std::string password = prompt_line("2FA password (visible): ");
          send(td_api::make_object<td_api::checkAuthenticationPassword>(password));
        },
        [](td_api::authorizationStateWaitOtherDeviceConfirmation& state) {
          std::cout << "confirm login from another Telegram client: " << state.link_ << '\n';
        },
        [](td_api::authorizationStateWaitRegistration&) {
          throw std::runtime_error("this CLI does not support new-account registration; log in with an existing account");
        },
        [](td_api::authorizationStateClosed&) { throw std::runtime_error("TDLib authorization closed"); },
        [](auto&) {}));
  }

  Config config_;
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_ = 0;
  std::uint64_t next_query_id_ = 1;
  bool is_authorized_ = false;
  td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
};

std::int64_t parse_int64_option(const ParsedArgs& args, const std::string& key) {
  const auto iterator = args.options.find(key);
  if (iterator == args.options.end() || iterator->second.empty()) {
    throw std::runtime_error("missing required option --" + key);
  }
  return std::stoll(iterator->second);
}

int parse_int_option(const ParsedArgs& args, const std::string& key, int fallback) {
  const auto iterator = args.options.find(key);
  return iterator == args.options.end() || iterator->second.empty() ? fallback : std::stoi(iterator->second);
}

std::optional<int> parse_optional_int_option(const ParsedArgs& args, const std::string& key) {
  const auto iterator = args.options.find(key);
  if (iterator == args.options.end() || iterator->second.empty()) {
    return std::nullopt;
  }
  return std::stoi(iterator->second);
}

std::string parse_string_option(const ParsedArgs& args, const std::string& key, std::string fallback) {
  const auto iterator = args.options.find(key);
  return iterator == args.options.end() ? std::move(fallback) : iterator->second;
}

void print_chats(TelegramClient& client, int limit) {
  auto result = client.request(td_api::make_object<td_api::getChats>(nullptr, limit), std::chrono::seconds(60));
  auto chats = td::move_tl_object_as<td_api::chats>(std::move(result));

  std::cout << "chat_id\ttype\ttitle\n";
  for (const auto chat_id : chats->chat_ids_) {
    auto chat_object = client.request(td_api::make_object<td_api::getChat>(chat_id), std::chrono::seconds(30));
    auto chat = td::move_tl_object_as<td_api::chat>(std::move(chat_object));
    std::cout << chat->id_ << '\t' << chat_type_label(*chat) << '\t' << chat->title_ << '\n';
  }
}

void load_all_chats(TelegramClient& client) {
  while (true) {
    auto result = client.request(td_api::make_object<td_api::loadChats>(nullptr, 100), std::chrono::seconds(60), true);
    if (result->get_id() == td_api::ok::ID) {
      continue;
    }
    if (result->get_id() == td_api::error::ID) {
      const auto& error = static_cast<const td_api::error&>(*result);
      if (error.code_ == 404) {
        return;
      }
      throw std::runtime_error("TDLib error " + std::to_string(error.code_) + ": " + error.message_);
    }
  }
}

void list_chats(TelegramClient& client, const ParsedArgs& args) {
  const std::optional<int> limit = parse_optional_int_option(args, "limit");
  if (limit) {
    print_chats(client, *limit);
    return;
  }

  load_all_chats(client);
  print_chats(client, std::numeric_limits<std::int32_t>::max());
}

void list_messages(TelegramClient& client, const ParsedArgs& args) {
  const std::int64_t chat_id = parse_int64_option(args, "chat");
  const std::optional<int> limit = parse_optional_int_option(args, "limit");
  const std::string json_path = parse_string_option(args, "json", "");
  std::ofstream json_output;
  std::int64_t from_message_id = 0;
  int printed = 0;

  if (json_path.empty()) {
    print_message_header();
  } else {
    json_output.open(json_path);
    if (!json_output) {
      throw std::runtime_error("failed to open JSON output file: " + json_path);
    }
    json_output << "[\n";
  }

  while (!limit || printed < *limit) {
    const int batch_limit = limit ? std::min(100, *limit - printed) : 100;
    auto result = client.request(
        td_api::make_object<td_api::getChatHistory>(chat_id, from_message_id, 0, batch_limit, false), std::chrono::seconds(60));
    auto messages = td::move_tl_object_as<td_api::messages>(std::move(result));
    if (messages->messages_.empty()) {
      break;
    }

    for (const auto& message : messages->messages_) {
      from_message_id = message->id_;
      if (json_output) {
        if (printed > 0) {
          json_output << ",\n";
        }
        write_message_json(json_output, *message);
      } else {
        print_message_row(*message);
      }
      ++printed;
      if (limit && printed >= *limit) {
        break;
      }
    }
  }

  if (json_output) {
    json_output << "\n]\n";
  }
}

void download_one_video(TelegramClient& client, const VideoFile& video, const std::filesystem::path& destination) {
  auto file_object = client.request(td_api::make_object<td_api::downloadFile>(video.file_id, 32, 0, 0, true), std::chrono::minutes(30));
  auto file = td::move_tl_object_as<td_api::file>(std::move(file_object));
  const std::filesystem::path local_path = file->local_->path_;
  if (local_path.empty() || !std::filesystem::exists(local_path)) {
    throw std::runtime_error("TDLib reported download complete but local file path is empty or missing");
  }

  save_downloaded_file(local_path, destination);
  client.request(td_api::make_object<td_api::deleteFile>(video.file_id), std::chrono::seconds(30));
  std::cout << "downloaded " << destination.string() << '\n';
}

void print_download_failures(const std::vector<DownloadFailure>& failures) {
  if (failures.empty()) {
    return;
  }

  std::cerr << "failed downloads:\n";
  for (const DownloadFailure& failure : failures) {
    std::cerr << "  message_id=" << failure.message_id << " file_id=" << failure.file_id
              << " destination=" << failure.destination.string() << " reason=" << failure.reason << '\n';
  }
}

void download_videos_parallel(TelegramClient& client, const std::vector<std::pair<VideoFile, PendingDownload>>& videos) {
  std::map<std::int32_t, PendingDownload> pending_by_file_id;
  std::size_t next_index = 0;
  int active = 0;
  std::vector<DownloadFailure> failures;

  auto start_next = [&]() {
    while (active < kDownloadParallel && next_index < videos.size()) {
      const VideoFile& video = videos[next_index].first;
      const PendingDownload& pending = videos[next_index].second;
      try {
        pending_by_file_id[video.file_id] = pending;
        client.send(td_api::make_object<td_api::downloadFile>(video.file_id, 32, 0, 0, false));
        ++active;
      } catch (const std::exception& error) {
        pending_by_file_id.erase(video.file_id);
        failures.push_back(DownloadFailure{pending.message_id, video.file_id, pending.destination, error.what()});
      }
      ++next_index;
    }
  };

  start_next();
  while (!pending_by_file_id.empty() || next_index < videos.size()) {
    auto response = client.receive(10.0);
    if (!response.object) {
      continue;
    }
    if (response.object->get_id() != td_api::updateFile::ID) {
      continue;
    }

    auto update = td::move_tl_object_as<td_api::updateFile>(std::move(response.object));
    const td_api::file& file = *update->file_;
    const auto pending = pending_by_file_id.find(file.id_);
    if (pending == pending_by_file_id.end()) {
      continue;
    }
    if (!file.local_->is_downloading_completed_) {
      continue;
    }
    if (file.local_->path_.empty() || !std::filesystem::exists(file.local_->path_)) {
      failures.push_back(DownloadFailure{
          pending->second.message_id, file.id_, pending->second.destination, "TDLib reported completion but local file is missing"});
      pending_by_file_id.erase(pending);
      --active;
      start_next();
      continue;
    }

    try {
      save_downloaded_file(file.local_->path_, pending->second.destination);
      client.send(td_api::make_object<td_api::deleteFile>(file.id_));
      std::cout << "downloaded " << pending->second.destination.string() << '\n';
    } catch (const std::exception& error) {
      failures.push_back(DownloadFailure{pending->second.message_id, file.id_, pending->second.destination, error.what()});
    }
    pending_by_file_id.erase(pending);
    --active;
    start_next();
  }

  print_download_failures(failures);
  if (!failures.empty()) {
    throw std::runtime_error(std::to_string(failures.size()) + " download(s) failed");
  }
}

void download_video(TelegramClient& client, const ParsedArgs& args) {
  const std::int64_t chat_id = parse_int64_option(args, "chat");
  const std::filesystem::path out = parse_string_option(args, "out", "downloads");
  const std::string messages = parse_string_option(args, "messages", "");

  if (!messages.empty()) {
    if (out.has_extension()) {
      throw std::runtime_error("--out must be a directory when using --messages");
    }
    std::vector<std::pair<VideoFile, PendingDownload>> videos;
    std::vector<DownloadFailure> failures;
    for (const std::int64_t message_id : parse_message_ids(messages)) {
      try {
        auto message_object = client.request(td_api::make_object<td_api::getMessage>(chat_id, message_id), std::chrono::seconds(30));
        auto message = td::move_tl_object_as<td_api::message>(std::move(message_object));
        std::optional<VideoFile> video = extract_video_file(*message);
        if (!video) {
          failures.push_back(DownloadFailure{message_id, 0, {}, "message does not contain a downloadable video"});
          continue;
        }
        videos.push_back({*video, PendingDownload{message_id, out / safe_file_name(*video, message_id)}});
      } catch (const std::exception& error) {
        failures.push_back(DownloadFailure{message_id, 0, {}, error.what()});
      }
    }
    print_download_failures(failures);
    download_videos_parallel(client, videos);
    if (!failures.empty()) {
      throw std::runtime_error(std::to_string(failures.size()) + " message(s) could not be prepared for download");
    }
    return;
  }

  const std::int64_t message_id = parse_int64_option(args, "message");
  auto message_object = client.request(td_api::make_object<td_api::getMessage>(chat_id, message_id), std::chrono::seconds(30));
  auto message = td::move_tl_object_as<td_api::message>(std::move(message_object));
  std::optional<VideoFile> video = extract_video_file(*message);
  if (!video) {
    throw std::runtime_error("selected message does not contain a downloadable video");
  }
  download_one_video(client, *video, download_destination(out, *video, message_id));
}

void upload_one_video(TelegramClient& client, std::int64_t chat_id, const std::filesystem::path& file_path, const std::string& caption) {
  if (file_path.empty() || !std::filesystem::exists(file_path)) {
    throw std::runtime_error("file does not exist: " + file_path.string());
  }

  auto local_file = td_api::make_object<td_api::inputFileLocal>();
  local_file->path_ = file_path.string();

  auto caption_text = td_api::make_object<td_api::formattedText>();
  caption_text->text_ = caption;

  auto video_content = td_api::make_object<td_api::inputMessageVideo>();
  video_content->video_ = std::move(local_file);
  video_content->supports_streaming_ = true;
  video_content->caption_ = std::move(caption_text);

  auto send_message = td_api::make_object<td_api::sendMessage>();
  send_message->chat_id_ = chat_id;
  send_message->input_message_content_ = std::move(video_content);

  auto result = client.request(std::move(send_message), std::chrono::minutes(30));
  auto message = td::move_tl_object_as<td_api::message>(std::move(result));
  std::cout << "uploaded message_id=" << message->id_ << '\n';
}

void print_upload_failures(const std::vector<UploadFailure>& failures) {
  if (failures.empty()) {
    return;
  }

  std::cerr << "failed uploads:\n";
  for (const UploadFailure& failure : failures) {
    std::cerr << "  name=" << failure.name.string() << " caption=" << failure.caption << " reason=" << failure.reason << '\n';
  }
}

void upload_video(TelegramClient& client, const ParsedArgs& args) {
  const std::int64_t chat_id = parse_int64_option(args, "chat");
  const std::string json_path = parse_string_option(args, "json", "");

  if (!json_path.empty()) {
    std::vector<UploadFailure> failures;
    for (const UploadItem& item : parse_upload_json_file(json_path)) {
      try {
        upload_one_video(client, chat_id, item.name, item.caption);
      } catch (const std::exception& error) {
        failures.push_back(UploadFailure{item.name, item.caption, error.what()});
      }
    }
    print_upload_failures(failures);
    if (!failures.empty()) {
      throw std::runtime_error(std::to_string(failures.size()) + " upload(s) failed");
    }
    return;
  }

  const std::filesystem::path file_path = parse_string_option(args, "file", "");
  const std::string caption = parse_string_option(args, "caption", "");
  upload_one_video(client, chat_id, file_path, caption);
}

void print_help() {
  std::cout << "tg-video-cli commands:\n"
            << "  login\n"
            << "  chats [--limit N]\n"
            << "  messages --chat CHAT_ID [--limit N] [--json FILE]\n"
            << "  download --chat CHAT_ID (--message MESSAGE_ID | --messages ID1,ID2) [--out PATH]\n"
            << "  upload --chat CHAT_ID (--file path.mp4 [--caption text] | --json FILE)\n";
}

ParsedArgs parse_args(int argc, char** argv) {
  ParsedArgs args;
  for (int arg_index = 1; arg_index < argc; ++arg_index) {
    std::string token = argv[arg_index];
    if (token.rfind("--", 0) == 0) {
      token = token.substr(2);
      const std::size_t equals_position = token.find('=');
      if (equals_position != std::string::npos) {
        args.options[token.substr(0, equals_position)] = token.substr(equals_position + 1);
      } else if (arg_index + 1 < argc && std::string(argv[arg_index + 1]).rfind("--", 0) != 0) {
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

}  // namespace

bool ParsedArgs::has_flag(const std::string& name) const {
  const auto iterator = options.find(name);
  return iterator != options.end() && iterator->second == "true";
}

int run_cli(int argc, char** argv) {
  const ParsedArgs args = parse_args(argc, argv);
  if (args.positional.empty() || args.positional.front() == "help" || args.has_flag("help")) {
    print_help();
    return 0;
  }

  const std::string command = args.positional.front();

  Config config = load_config("config/telegram.conf");
  TelegramClient client(std::move(config));
  client.ensure_authorized();

  if (command == "login") {
    std::cout << "authorized\n";
  } else if (command == "chats") {
    list_chats(client, args);
  } else if (command == "messages") {
    list_messages(client, args);
  } else if (command == "download") {
    download_video(client, args);
  } else if (command == "upload") {
    upload_video(client, args);
  } else {
    throw std::runtime_error("unknown command: " + command);
  }
  return 0;
}

}  // namespace tg_video_cli

int main(int argc, char** argv) {
  try {
    return tg_video_cli::run_cli(argc, argv);
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}