#ifndef TG_TOOLS_SRC_MESSAGE_FORMAT_H_
#define TG_TOOLS_SRC_MESSAGE_FORMAT_H_

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

#include <td/telegram/td_api.h>

namespace tg_tools {

struct VideoFile {
  int file_id = 0;
  std::string file_name;
  std::string mime_type;
};

std::string ChatTypeLabel(const td::td_api::chat& chat);
std::optional<VideoFile> ExtractVideoFile(const td::td_api::message& message);
void PrintMessageHeader();
void PrintMessageRow(const td::td_api::message& message);
void WriteMessageJson(std::ostream& output, const td::td_api::message& message);
std::string SafeFileName(const VideoFile& video, std::int64_t message_id);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_MESSAGE_FORMAT_H_
