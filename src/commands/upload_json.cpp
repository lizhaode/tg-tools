#include "commands/upload_json.h"

#include <fstream>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "core/text_util.h"

namespace tg_tools {
namespace {

using Json = nlohmann::json;

bool ReadTextFile(const std::filesystem::path& path, std::string* text,
                  std::string* error) {
  if (text == nullptr) {
    return SetError(error, "内部错误：文本输出指针为空");
  }
  std::ifstream input(path);
  if (!input) {
    return SetError(error, "无法打开文件：" + path.string());
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  *text = buffer.str();
  return true;
}

bool ParseUploadObject(const Json& object, std::size_t index, UploadItem* item,
                       std::string* error) {
  if (item == nullptr) {
    return SetError(error, "内部错误：上传对象输出指针为空");
  }
  if (!object.is_object()) {
    return SetError(
        error, "上传 JSON 第 " + std::to_string(index + 1) + " 项必须是对象");
  }
  if (!object.contains("name")) {
    return SetError(
        error, "上传 JSON 第 " + std::to_string(index + 1) + " 项缺少 name");
  }
  if (!object.at("name").is_string()) {
    return SetError(error, "上传 JSON 第 " + std::to_string(index + 1) +
                               " 项的 name 必须是字符串");
  }
  if (object.contains("caption") && !object.at("caption").is_string()) {
    return SetError(error, "上传 JSON 第 " + std::to_string(index + 1) +
                               " 项的 caption 必须是字符串");
  }

  *item = UploadItem{};
  item->name = object.at("name").get<std::string>();
  item->caption = object.value("caption", "");
  if (item->name.empty()) {
    return SetError(error, "上传 JSON 第 " + std::to_string(index + 1) +
                               " 项的 name 不能为空");
  }
  return true;
}

}  // namespace

bool ParseUploadJsonFile(const std::filesystem::path& path,
                         std::vector<UploadItem>* items, std::string* error) {
  if (items == nullptr) {
    return SetError(error, "内部错误：上传列表输出指针为空");
  }
  std::string text;
  if (!ReadTextFile(path, &text, error)) {
    return false;
  }

  Json root = Json::parse(text, nullptr, false);
  if (root.is_discarded()) {
    return SetError(error, "上传 JSON 解析失败：文件内容不是有效 JSON");
  }

  if (!root.is_array()) {
    return SetError(error, "上传 JSON 必须是数组");
  }

  items->clear();
  for (std::size_t index = 0; index < root.size(); ++index) {
    UploadItem item;
    if (!ParseUploadObject(root[index], index, &item, error)) {
      return false;
    }
    items->push_back(std::move(item));
  }

  if (items->empty()) {
    return SetError(error, "上传 JSON 没有任何项目");
  }
  return true;
}

}  // namespace tg_tools
