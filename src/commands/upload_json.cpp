#include "commands/upload_json.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <utility>

#include "core/text_util.h"

namespace tg_tools {
namespace {

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

void SkipJsonSpace(const std::string& text, std::size_t& index) {
  while (index < text.size() &&
         std::isspace(static_cast<unsigned char>(text[index]))) {
    ++index;
  }
}

bool ParseJsonStringValue(const std::string& text, std::size_t* index,
                          std::string* value, std::string* error) {
  if (index == nullptr || value == nullptr) {
    return SetError(error, "内部错误：JSON 字符串输出指针为空");
  }
  value->clear();
  SkipJsonSpace(text, *index);
  if (*index >= text.size() || text[*index] != '"') {
    return SetError(error, "JSON 需要字符串值");
  }
  ++(*index);

  while (*index < text.size()) {
    const char character = text[(*index)++];
    if (character == '"') {
      return true;
    }
    if (character != '\\') {
      value->push_back(character);
      continue;
    }
    if (*index >= text.size()) {
      return SetError(error, "JSON 转义不完整");
    }
    const char escaped = text[(*index)++];
    switch (escaped) {
      case '"':
        value->push_back('"');
        break;
      case '\\':
        value->push_back('\\');
        break;
      case 'n':
        value->push_back('\n');
        break;
      case 'r':
        value->push_back('\r');
        break;
      case 't':
        value->push_back('\t');
        break;
      default:
        value->push_back(escaped);
        break;
    }
  }
  return SetError(error, "JSON 字符串未结束");
}

bool ParseUploadObject(const std::string& text, std::size_t* index,
                       UploadItem* item, std::string* error) {
  if (index == nullptr || item == nullptr) {
    return SetError(error, "内部错误：上传对象输出指针为空");
  }
  SkipJsonSpace(text, *index);
  if (*index >= text.size() || text[*index] != '{') {
    return SetError(error, "上传 JSON 项必须是对象");
  }
  ++(*index);
  *item = UploadItem{};
  while (true) {
    SkipJsonSpace(text, *index);
    if (*index < text.size() && text[*index] == '}') {
      ++(*index);
      break;
    }

    std::string key;
    if (!ParseJsonStringValue(text, index, &key, error)) {
      return false;
    }
    SkipJsonSpace(text, *index);
    if (*index >= text.size() || text[*index] != ':') {
      return SetError(error, "上传 JSON 对象缺少 ':'");
    }
    ++(*index);
    std::string value;
    if (!ParseJsonStringValue(text, index, &value, error)) {
      return false;
    }

    if (key == "name") {
      item->name = value;
    } else if (key == "caption") {
      item->caption = value;
    }

    SkipJsonSpace(text, *index);
    if (*index < text.size() && text[*index] == ',') {
      ++(*index);
      continue;
    }
    if (*index < text.size() && text[*index] == '}') {
      ++(*index);
      break;
    }
    return SetError(error, "上传 JSON 对象缺少 ',' 或 '}'");
  }

  if (item->name.empty()) {
    return SetError(error, "上传 JSON 对象缺少 name");
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
  std::size_t index = 0;
  items->clear();
  SkipJsonSpace(text, index);

  if (index >= text.size() || text[index] != '[') {
    return SetError(error, "上传 JSON 必须是数组");
  }
  ++index;

  while (true) {
    SkipJsonSpace(text, index);
    if (index < text.size() && text[index] == ']') {
      ++index;
      break;
    }
    UploadItem item;
    if (!ParseUploadObject(text, &index, &item, error)) {
      return false;
    }
    items->push_back(std::move(item));
    SkipJsonSpace(text, index);
    if (index < text.size() && text[index] == ',') {
      ++index;
    }
  }

  if (items->empty()) {
    return SetError(error, "上传 JSON 没有任何项目");
  }
  return true;
}

}  // namespace tg_tools
