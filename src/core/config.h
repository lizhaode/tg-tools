#ifndef TG_TOOLS_SRC_CORE_CONFIG_H_
#define TG_TOOLS_SRC_CORE_CONFIG_H_

#include <filesystem>
#include <string>

namespace tg_tools {

struct Config {
  int api_id = 0;
  std::string api_hash;
  std::string phone_number;
  std::string database_directory = "tdlib-db";
  std::string files_directory = "tdlib-files";
  std::string database_encryption_key;
  std::string system_language_code = "zh-CN";
  std::string device_model = "CLI";
  std::string system_version;
  std::string application_version = "0.1.0";
};

bool LoadConfig(const std::filesystem::path& path, Config* config,
                std::string* error);
bool ValidateConfig(const Config& config, std::string* error);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_CORE_CONFIG_H_
