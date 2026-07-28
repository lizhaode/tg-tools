#ifndef TG_TOOLS_SRC_UPLOAD_JSON_H_
#define TG_TOOLS_SRC_UPLOAD_JSON_H_

#include <filesystem>
#include <string>
#include <vector>

namespace tg_tools {

struct UploadItem {
  std::filesystem::path name;
  std::string caption;
};

bool ParseUploadJsonFile(const std::filesystem::path& path,
                         std::vector<UploadItem>* items, std::string* error);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_UPLOAD_JSON_H_
