#include "core/config.h"

#include <fstream>
#include <map>

#include "core/text_util.h"

namespace tg_tools {
namespace {

bool ReadKeyValueFile(const std::filesystem::path& path,
                      std::map<std::string, std::string>* values,
                      std::string* error) {
  if (values == nullptr) {
    return SetError(error, "内部错误：配置输出指针为空");
  }
  values->clear();
  if (!std::filesystem::exists(path)) {
    return true;
  }

  std::ifstream input(path);
  if (!input) {
    return SetError(error, "无法打开配置文件：" + path.string());
  }

  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty() || line.front() == '#') {
      continue;
    }
    const auto separator = line.find('=');
    if (separator == std::string::npos) {
      continue;
    }
    (*values)[Trim(line.substr(0, separator))] =
        Trim(line.substr(separator + 1));
  }
  return true;
}

std::string MapStringOr(const std::map<std::string, std::string>& values,
                        const std::string& key, const std::string& fallback) {
  const auto iterator = values.find(key);
  return iterator == values.end() ? fallback : iterator->second;
}

bool MapIntOr(const std::map<std::string, std::string>& values,
              const std::string& key, int fallback, int* value,
              std::string* error) {
  const auto iterator = values.find(key);
  if (iterator == values.end() || iterator->second.empty()) {
    *value = fallback;
    return true;
  }
  if (!ParseIntText(iterator->second, value)) {
    return SetError(error, "配置项 " + key + " 必须是整数");
  }
  return true;
}

}  // namespace

bool LoadConfig(const std::filesystem::path& path, Config* config,
                std::string* error) {
  if (config == nullptr) {
    return SetError(error, "内部错误：配置输出指针为空");
  }
  std::map<std::string, std::string> values;
  if (!ReadKeyValueFile(path, &values, error)) {
    return false;
  }
  if (!MapIntOr(values, "api_id", config->api_id, &config->api_id, error)) {
    return false;
  }
  config->api_hash = MapStringOr(values, "api_hash", config->api_hash);
  config->phone_number =
      MapStringOr(values, "phone_number", config->phone_number);
  config->database_directory =
      MapStringOr(values, "database_directory", config->database_directory);
  config->files_directory =
      MapStringOr(values, "files_directory", config->files_directory);
  config->database_encryption_key = MapStringOr(
      values, "database_encryption_key", config->database_encryption_key);
  config->system_language_code =
      MapStringOr(values, "system_language_code", config->system_language_code);
  config->device_model =
      MapStringOr(values, "device_model", config->device_model);
  config->system_version =
      MapStringOr(values, "system_version", config->system_version);
  config->application_version =
      MapStringOr(values, "application_version", config->application_version);
  return true;
}

bool ValidateConfig(const Config& config, std::string* error) {
  if (config.api_id <= 0 || config.api_hash.empty() ||
      config.api_hash == "PUT_YOUR_API_HASH_HERE") {
    return SetError(
        error,
        "缺少 Telegram api_id/api_hash。请先复制 config/telegram.example.conf "
        "为 "
        "config/telegram.conf 并填写配置。");
  }
  return true;
}

}  // namespace tg_tools
