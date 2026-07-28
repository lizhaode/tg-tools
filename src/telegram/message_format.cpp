#include "telegram/message_format.h"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <ostream>

#include "core/text_util.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

constexpr std::size_t kMessageIdColumnWidth = 14;
constexpr std::size_t kDateColumnWidth = 17;
constexpr std::size_t kTypeColumnWidth = 10;
constexpr std::size_t kFileColumnWidth = 124;
constexpr std::size_t kFileClipWidth = 120;
constexpr std::size_t kTextClipWidth = 120;

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

MessageRow DescribeMessage(const td_api::message& message) {
  MessageRow row;

  switch (message.content_->get_id()) {
    case td_api::messageText::ID: {
      const auto& content =
          static_cast<const td_api::messageText&>(*message.content_);
      row.type = "text";
      row.text = content.text_->text_;
      break;
    }
    case td_api::messagePhoto::ID: {
      const auto& content =
          static_cast<const td_api::messagePhoto&>(*message.content_);
      row.type = "photo";
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    case td_api::messageVideo::ID: {
      const auto& content =
          static_cast<const td_api::messageVideo&>(*message.content_);
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
      const auto& content =
          static_cast<const td_api::messageVideoNote&>(*message.content_);
      row.type = "video_note";
      row.file_id = content.video_note_->video_->id_;
      row.mime_type = "video/mp4";
      row.duration = content.video_note_->duration_;
      row.width = content.video_note_->length_;
      row.height = content.video_note_->length_;
      break;
    }
    case td_api::messageDocument::ID: {
      const auto& content =
          static_cast<const td_api::messageDocument&>(*message.content_);
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
      const auto& content =
          static_cast<const td_api::messageAnimation&>(*message.content_);
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

std::string DefaultExtension(const std::string& mime_type) {
  if (mime_type == "video/quicktime") {
    return ".mov";
  }
  if (mime_type == "video/x-matroska") {
    return ".mkv";
  }
  return ".mp4";
}

}  // namespace

std::string ChatTypeLabel(const td_api::chat& chat) {
  switch (chat.type_->get_id()) {
    case td_api::chatTypeBasicGroup::ID:
      return "basic_group";
    case td_api::chatTypeSupergroup::ID: {
      const auto& supergroup =
          static_cast<const td_api::chatTypeSupergroup&>(*chat.type_);
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

std::optional<VideoFile> ExtractVideoFile(const td_api::message& message) {
  switch (message.content_->get_id()) {
    case td_api::messageVideo::ID: {
      const auto& content =
          static_cast<const td_api::messageVideo&>(*message.content_);
      return VideoFile{content.video_->video_->id_, content.video_->file_name_,
                       content.video_->mime_type_};
    }
    case td_api::messageVideoNote::ID: {
      const auto& content =
          static_cast<const td_api::messageVideoNote&>(*message.content_);
      return VideoFile{content.video_note_->video_->id_, "", "video/mp4"};
    }
    case td_api::messageDocument::ID: {
      const auto& content =
          static_cast<const td_api::messageDocument&>(*message.content_);
      if (content.document_->mime_type_.rfind("video/", 0) == 0) {
        return VideoFile{content.document_->document_->id_,
                         content.document_->file_name_,
                         content.document_->mime_type_};
      }
      return std::nullopt;
    }
    default:
      return std::nullopt;
  }
}

void PrintMessageHeader() {
  std::cout << PadRight("message_id", kMessageIdColumnWidth)
            << PadRight("date", kDateColumnWidth)
            << PadRight("type", kTypeColumnWidth)
            << PadRight("file", kFileColumnWidth) << "text\n";
}

void PrintMessageRow(const td_api::message& message) {
  const MessageRow row = DescribeMessage(message);
  const std::string file_name = ClipDisplay(row.file_name, kFileClipWidth);
  std::cout << PadRight(std::to_string(message.id_), kMessageIdColumnWidth)
            << PadRight(FormatTimestamp(message.date_), kDateColumnWidth)
            << PadRight(row.type, kTypeColumnWidth)
            << PadRight(file_name, kFileColumnWidth)
            << ClipDisplay(row.text, kTextClipWidth) << '\n';
}

void WriteMessageJson(std::ostream& output, const td_api::message& message) {
  const MessageRow row = DescribeMessage(message);
  output << "  {";
  output << "\"message_id\":" << message.id_;
  output << ",\"date\":" << message.date_;
  output << ",\"date_text\":\"" << JsonEscape(FormatTimestamp(message.date_))
         << "\"";
  output << ",\"type\":\"" << JsonEscape(row.type) << "\"";
  output << ",\"file_id\":" << row.file_id;
  output << ",\"file_name\":\"" << JsonEscape(row.file_name) << "\"";
  output << ",\"mime_type\":\"" << JsonEscape(row.mime_type) << "\"";
  output << ",\"duration\":" << row.duration;
  output << ",\"width\":" << row.width;
  output << ",\"height\":" << row.height;
  output << ",\"text\":\"" << JsonEscape(row.text) << "\"";
  output << "}";
}

std::string SafeFileName(const VideoFile& video, std::int64_t message_id) {
  std::filesystem::path candidate(video.file_name);
  std::string file_name = candidate.filename().string();
  if (file_name.empty()) {
    file_name = "telegram_video_" + std::to_string(message_id) +
                DefaultExtension(video.mime_type);
  }
  return file_name;
}

}  // namespace tg_tools
