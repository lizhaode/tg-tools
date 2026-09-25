#ifndef TG_TOOLS_SRC_TELEGRAM_MESSAGE_FORMAT_H_
#define TG_TOOLS_SRC_TELEGRAM_MESSAGE_FORMAT_H_

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include <td/telegram/td_api.h>

namespace tg_tools {

struct VideoFile {
  int file_id = 0;
  std::string file_name;
  std::string mime_type;
  int width = 0;
  int height = 0;
  std::int64_t size = 0;
};

struct AlternativeVideo {
  int file_id = 0;
  int width = 0;
  int height = 0;
  std::string codec;
  std::int64_t size = 0;
  int hls_file_id = 0;
};

struct MessageRow {
  std::int64_t message_id = 0;
  std::int64_t date = 0;
  std::string type;
  std::string file_name;
  std::string text;
  std::string mime_type;
  std::string album_position;
  std::int32_t file_id = 0;
  std::int32_t duration = 0;
  std::int32_t width = 0;
  std::int32_t height = 0;
  std::int64_t size = 0;
  bool supports_streaming = false;
  bool has_stickers = false;
  std::vector<AlternativeVideo> alternative_videos;
};

std::string ChatTypeName(const td::td_api::chat& chat);
std::optional<VideoFile> ExtractVideoFile(
    const td::td_api::message& message,
    std::vector<AlternativeVideo>* alternatives = nullptr);
VideoFile SelectBestQuality(const VideoFile& original,
                            const std::vector<AlternativeVideo>& alternatives);
std::vector<MessageRow> DescribeMessages(
    const std::vector<td::td_api::object_ptr<td::td_api::message>>& messages);
void PrintMessageHeader();
void PrintMessageRow(const MessageRow& row);
void WriteMessageCsv(std::ostream& output, const MessageRow& row);
std::string SafeFileName(const VideoFile& video, std::int64_t message_id);

}  // namespace tg_tools

#endif  // TG_TOOLS_SRC_TELEGRAM_MESSAGE_FORMAT_H_
