#include "telegram/message_format.h"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "core/text_util.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

constexpr std::size_t kMessageIdColumnWidth = 14;
constexpr std::size_t kDateColumnWidth = 17;
constexpr std::size_t kTypeColumnWidth = 32;
constexpr std::size_t kFileColumnWidth = 40;
constexpr std::size_t kQualityColumnWidth = 38;
constexpr std::size_t kFileClipWidth = 40;
constexpr std::size_t kTextClipWidth = 120;

// 由 TDLib 生成的 td_api.h 中全部 message* 类型生成，类型名与 ID 一一对应。
// TDLib 升级后需重新生成；TDLib 未提供只取类型名的轻量接口（to_string 是
// 整棵对象树的调试转储，且 store 非虚、release 下可能被 LOG_IS_STRIPPED
// 剥离）。
std::string MessageTypeName(const td_api::MessageContent& content) {
  switch (content.get_id()) {
    case td_api::messageVenue::ID:
      return "messageVenue";
    case td_api::messageGift::ID:
      return "messageGift";
    case td_api::messageAnimation::ID:
      return "messageAnimation";
    case td_api::messageVideoChatScheduled::ID:
      return "messageVideoChatScheduled";
    case td_api::messageUnsupported::ID:
      return "messageUnsupported";
    case td_api::messageUpgradedGiftPurchaseOffer::ID:
      return "messageUpgradedGiftPurchaseOffer";
    case td_api::messagePollOptionDeleted::ID:
      return "messagePollOptionDeleted";
    case td_api::messageManagedBotCreated::ID:
      return "messageManagedBotCreated";
    case td_api::messageForumTopicIsHiddenToggled::ID:
      return "messageForumTopicIsHiddenToggled";
    case td_api::messageBotWriteAccessAllowed::ID:
      return "messageBotWriteAccessAllowed";
    case td_api::messageScreenshotTaken::ID:
      return "messageScreenshotTaken";
    case td_api::messagePhoto::ID:
      return "messagePhoto";
    case td_api::messageContactRegistered::ID:
      return "messageContactRegistered";
    case td_api::messageInviteVideoChatParticipants::ID:
      return "messageInviteVideoChatParticipants";
    case td_api::messageGiveawayPrizeStars::ID:
      return "messageGiveawayPrizeStars";
    case td_api::messageUpgradedGift::ID:
      return "messageUpgradedGift";
    case td_api::messageExpiredPhoto::ID:
      return "messageExpiredPhoto";
    case td_api::messagePassportDataReceived::ID:
      return "messagePassportDataReceived";
    case td_api::messageChatShared::ID:
      return "messageChatShared";
    case td_api::messageSuggestProfilePhoto::ID:
      return "messageSuggestProfilePhoto";
    case td_api::messageRichMessage::ID:
      return "messageRichMessage";
    case td_api::messageExpiredVideo::ID:
      return "messageExpiredVideo";
    case td_api::messageChatAddedToCommunity::ID:
      return "messageChatAddedToCommunity";
    case td_api::messagePaymentSuccessfulBot::ID:
      return "messagePaymentSuccessfulBot";
    case td_api::messageUsersShared::ID:
      return "messageUsersShared";
    case td_api::messageChatChangePhoto::ID:
      return "messageChatChangePhoto";
    case td_api::messageChatHasProtectedContentToggled::ID:
      return "messageChatHasProtectedContentToggled";
    case td_api::messagePaidMedia::ID:
      return "messagePaidMedia";
    case td_api::messageChecklistTasksDone::ID:
      return "messageChecklistTasksDone";
    case td_api::messageForumTopicCreated::ID:
      return "messageForumTopicCreated";
    case td_api::messageSuggestBirthdate::ID:
      return "messageSuggestBirthdate";
    case td_api::messageLiveLocation::ID:
      return "messageLiveLocation";
    case td_api::messageRefundedUpgradedGift::ID:
      return "messageRefundedUpgradedGift";
    case td_api::messageSuggestedPostPaid::ID:
      return "messageSuggestedPostPaid";
    case td_api::messageSuggestedPostRefunded::ID:
      return "messageSuggestedPostRefunded";
    case td_api::messageContact::ID:
      return "messageContact";
    case td_api::messageGiveawayCompleted::ID:
      return "messageGiveawayCompleted";
    case td_api::messageSticker::ID:
      return "messageSticker";
    case td_api::messageSupergroupChatCreate::ID:
      return "messageSupergroupChatCreate";
    case td_api::messageGiveaway::ID:
      return "messageGiveaway";
    case td_api::messagePaidMessagePriceChanged::ID:
      return "messagePaidMessagePriceChanged";
    case td_api::messageChatDeletePhoto::ID:
      return "messageChatDeletePhoto";
    case td_api::messageWebAppDataSent::ID:
      return "messageWebAppDataSent";
    case td_api::messageGame::ID:
      return "messageGame";
    case td_api::messageSuggestedPostDeclined::ID:
      return "messageSuggestedPostDeclined";
    case td_api::messageWebAppDataReceived::ID:
      return "messageWebAppDataReceived";
    case td_api::messageForumTopicEdited::ID:
      return "messageForumTopicEdited";
    case td_api::messageUpgradedGiftPurchaseOfferRejected::ID:
      return "messageUpgradedGiftPurchaseOfferRejected";
    case td_api::messageProximityAlertTriggered::ID:
      return "messageProximityAlertTriggered";
    case td_api::messageChatUpgradeTo::ID:
      return "messageChatUpgradeTo";
    case td_api::messageCall::ID:
      return "messageCall";
    case td_api::messageExpiredVoiceNote::ID:
      return "messageExpiredVoiceNote";
    case td_api::messageLocation::ID:
      return "messageLocation";
    case td_api::messageInvoice::ID:
      return "messageInvoice";
    case td_api::messageAudio::ID:
      return "messageAudio";
    case td_api::messagePaymentRefunded::ID:
      return "messagePaymentRefunded";
    case td_api::messageChatUpgradeFrom::ID:
      return "messageChatUpgradeFrom";
    case td_api::messageChatHasProtectedContentDisableRequested::ID:
      return "messageChatHasProtectedContentDisableRequested";
    case td_api::messageChatOwnerLeft::ID:
      return "messageChatOwnerLeft";
    case td_api::messageVideoChatStarted::ID:
      return "messageVideoChatStarted";
    case td_api::messageVoiceNote::ID:
      return "messageVoiceNote";
    case td_api::messageSuggestedPostApproved::ID:
      return "messageSuggestedPostApproved";
    case td_api::messagePaidMessagesRefunded::ID:
      return "messagePaidMessagesRefunded";
    case td_api::messageChatRemovedFromCommunity::ID:
      return "messageChatRemovedFromCommunity";
    case td_api::messageDocument::ID:
      return "messageDocument";
    case td_api::messageExpiredVideoNote::ID:
      return "messageExpiredVideoNote";
    case td_api::messageChecklist::ID:
      return "messageChecklist";
    case td_api::messagePremiumGiftCode::ID:
      return "messagePremiumGiftCode";
    case td_api::messageChatChangeTitle::ID:
      return "messageChatChangeTitle";
    case td_api::messageBasicGroupChatCreate::ID:
      return "messageBasicGroupChatCreate";
    case td_api::messageGiftedPremium::ID:
      return "messageGiftedPremium";
    case td_api::messageStakeDice::ID:
      return "messageStakeDice";
    case td_api::messagePoll::ID:
      return "messagePoll";
    case td_api::messageAnimatedEmoji::ID:
      return "messageAnimatedEmoji";
    case td_api::messageChatDeleteMember::ID:
      return "messageChatDeleteMember";
    case td_api::messageGiftedTon::ID:
      return "messageGiftedTon";
    case td_api::messageVideo::ID:
      return "messageVideo";
    case td_api::messagePinMessage::ID:
      return "messagePinMessage";
    case td_api::messageVideoNote::ID:
      return "messageVideoNote";
    case td_api::messageGiveawayCreated::ID:
      return "messageGiveawayCreated";
    case td_api::messagePassportDataSent::ID:
      return "messagePassportDataSent";
    case td_api::messageChatSetBackground::ID:
      return "messageChatSetBackground";
    case td_api::messagePaymentSuccessful::ID:
      return "messagePaymentSuccessful";
    case td_api::messageGiftedStars::ID:
      return "messageGiftedStars";
    case td_api::messageDice::ID:
      return "messageDice";
    case td_api::messageChatJoinByRequest::ID:
      return "messageChatJoinByRequest";
    case td_api::messageChatSetTheme::ID:
      return "messageChatSetTheme";
    case td_api::messageForumTopicIsClosedToggled::ID:
      return "messageForumTopicIsClosedToggled";
    case td_api::messageDirectMessagePriceChanged::ID:
      return "messageDirectMessagePriceChanged";
    case td_api::messageGameScore::ID:
      return "messageGameScore";
    case td_api::messageCustomServiceAction::ID:
      return "messageCustomServiceAction";
    case td_api::messageStory::ID:
      return "messageStory";
    case td_api::messageChatBoost::ID:
      return "messageChatBoost";
    case td_api::messageChatSetMessageAutoDeleteTime::ID:
      return "messageChatSetMessageAutoDeleteTime";
    case td_api::messageChatOwnerChanged::ID:
      return "messageChatOwnerChanged";
    case td_api::messageChatAddMembers::ID:
      return "messageChatAddMembers";
    case td_api::messageGroupCall::ID:
      return "messageGroupCall";
    case td_api::messageText::ID:
      return "messageText";
    case td_api::messageSuggestedPostApprovalFailed::ID:
      return "messageSuggestedPostApprovalFailed";
    case td_api::messagePollOptionAdded::ID:
      return "messagePollOptionAdded";
    case td_api::messageChatJoinByLink::ID:
      return "messageChatJoinByLink";
    case td_api::messageVideoChatEnded::ID:
      return "messageVideoChatEnded";
    case td_api::messageChecklistTasksAdded::ID:
      return "messageChecklistTasksAdded";
    case td_api::messageGiveawayWinners::ID:
      return "messageGiveawayWinners";
    default:
      return "type_" + std::to_string(content.get_id());
  }
}

MessageRow DescribeMessage(const td_api::message& message) {
  MessageRow row;
  row.message_id = message.id_;
  row.date = message.date_;
  row.type = MessageTypeName(*message.content_);

  switch (message.content_->get_id()) {
    case td_api::messageText::ID: {
      const auto& content =
          static_cast<const td_api::messageText&>(*message.content_);
      row.text = content.text_->text_;
      break;
    }
    case td_api::messagePhoto::ID: {
      const auto& content =
          static_cast<const td_api::messagePhoto&>(*message.content_);
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    case td_api::messageVideo::ID: {
      const auto& content =
          static_cast<const td_api::messageVideo&>(*message.content_);
      row.file_id = content.video_->video_->id_;
      row.file_name = content.video_->file_name_;
      row.mime_type = content.video_->mime_type_;
      row.duration = content.video_->duration_;
      row.width = content.video_->width_;
      row.height = content.video_->height_;
      row.size = content.video_->video_->size_ > 0
                     ? content.video_->video_->size_
                     : content.video_->video_->expected_size_;
      row.supports_streaming = content.video_->supports_streaming_;
      row.has_stickers = content.video_->has_stickers_;
      for (const auto& alternative : content.alternative_videos_) {
        row.alternative_videos.push_back(AlternativeVideo{
            alternative->video_->id_, alternative->width_, alternative->height_,
            alternative->codec_,
            alternative->video_->size_ > 0
                ? alternative->video_->size_
                : alternative->video_->expected_size_,
            alternative->hls_file_ != nullptr ? alternative->hls_file_->id_
                                              : 0});
      }
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    case td_api::messageVideoNote::ID: {
      const auto& content =
          static_cast<const td_api::messageVideoNote&>(*message.content_);
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
      if (content.caption_) {
        row.text = content.caption_->text_;
      }
      break;
    }
    default:
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

// 输出 TDLib 原始类型名，与 MessageTypeName 保持一致。
// 注意：channel 与 supergroup 在 TDLib 中同为 chatTypeSupergroup，
// 原始名不再区分二者。
std::string ChatTypeName(const td_api::chat& chat) {
  switch (chat.type_->get_id()) {
    case td_api::chatTypeBasicGroup::ID:
      return "chatTypeBasicGroup";
    case td_api::chatTypeSupergroup::ID:
      return "chatTypeSupergroup";
    case td_api::chatTypePrivate::ID:
      return "chatTypePrivate";
    case td_api::chatTypeSecret::ID:
      return "chatTypeSecret";
    default:
      return "type_" + std::to_string(chat.type_->get_id());
  }
}

std::optional<VideoFile> ExtractVideoFile(
    const td_api::message& message,
    std::vector<AlternativeVideo>* alternatives) {
  switch (message.content_->get_id()) {
    case td_api::messageVideo::ID: {
      const auto& content =
          static_cast<const td_api::messageVideo&>(*message.content_);
      VideoFile video{content.video_->video_->id_,
                      content.video_->file_name_,
                      content.video_->mime_type_,
                      content.video_->width_,
                      content.video_->height_,
                      content.video_->video_->size_ > 0
                          ? content.video_->video_->size_
                          : content.video_->video_->expected_size_};
      if (alternatives != nullptr) {
        alternatives->clear();
        for (const auto& alternative : content.alternative_videos_) {
          alternatives->push_back(AlternativeVideo{
              alternative->video_->id_, alternative->width_,
              alternative->height_, alternative->codec_,
              alternative->video_->size_ > 0
                  ? alternative->video_->size_
                  : alternative->video_->expected_size_,
              alternative->hls_file_ != nullptr ? alternative->hls_file_->id_
                                                : 0});
        }
      }
      return video;
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

VideoFile SelectBestQuality(const VideoFile& original,
                            const std::vector<AlternativeVideo>& alternatives) {
  const AlternativeVideo* best = nullptr;
  for (const AlternativeVideo& alternative : alternatives) {
    if (alternative.height <= 0) {
      continue;
    }
    if (best == nullptr || alternative.height > best->height) {
      best = &alternative;
    }
  }
  if (best == nullptr || best->height <= original.height) {
    return original;
  }

  VideoFile selected = original;
  selected.file_id = best->file_id;
  selected.width = best->width;
  selected.height = best->height;
  selected.size = best->size;
  const std::filesystem::path path(original.file_name);
  const std::string stem = path.stem().string();
  const std::string extension = path.extension().string();
  selected.file_name =
      stem + "_" + std::to_string(best->height) + "p" + extension;
  return selected;
}

std::vector<std::string> QualityColumnLines(const MessageRow& row) {
  std::vector<std::string> lines;
  if (row.width > 0 && row.height > 0) {
    lines.push_back(std::to_string(row.height) + "p " +
                    FormatSizeShort(row.size));
  }
  for (const AlternativeVideo& alternative : row.alternative_videos) {
    lines.push_back(std::to_string(alternative.height) + "p " +
                    FormatSizeShort(alternative.size));
  }
  return lines;
}

void PrintMessageHeader() {
  std::cout << PadLeft("message_id", kMessageIdColumnWidth) << ' '
            << PadRight("date", kDateColumnWidth) << ' '
            << PadRight("type", kTypeColumnWidth) << ' '
            << PadRight("file", kFileColumnWidth) << ' '
            << PadRight("quality", kQualityColumnWidth) << " text\n";
}

// 提取媒体组关系：在 album_position 中记录组内序号，caption 保留在原消息
// 上（组内通常只有最后一条有），不向其他成员复制。
std::vector<MessageRow> DescribeMessages(
    const std::vector<td_api::object_ptr<td_api::message>>& messages) {
  std::vector<MessageRow> rows;
  rows.reserve(messages.size());
  std::map<std::int64_t, std::vector<std::size_t>> album_members;
  for (std::size_t index = 0; index < messages.size(); ++index) {
    const auto& message = messages[index];
    rows.push_back(DescribeMessage(*message));
    if (message->media_album_id_ != 0) {
      album_members[message->media_album_id_].push_back(index);
    }
  }
  for (const auto& member : album_members) {
    const std::vector<std::size_t>& indexes = member.second;
    const std::string size_text = std::to_string(indexes.size());
    for (std::size_t position = 0; position < indexes.size(); ++position) {
      rows[indexes[position]].album_position =
          std::to_string(position + 1) + "/" + size_text;
    }
  }
  return rows;
}

void PrintMessageRow(const MessageRow& row) {
  std::string file_display = row.file_name;
  if (row.duration > 0) {
    file_display += " (" + std::to_string(row.duration) + "s)";
  }
  if (file_display.empty() && !row.album_position.empty()) {
    file_display = "组内 " + row.album_position;
  }

  const std::string prefix =
      PadLeft(std::to_string(row.message_id), kMessageIdColumnWidth) + ' ' +
      PadRight(FormatTimestamp(row.date), kDateColumnWidth) + ' ' +
      PadRight(ClipDisplay(row.type, kTypeColumnWidth), kTypeColumnWidth) +
      ' ' +
      PadRight(ClipDisplay(file_display, kFileClipWidth), kFileColumnWidth);
  const std::vector<std::string> quality_lines = QualityColumnLines(row);
  const std::string text = ClipDisplay(row.text, kTextClipWidth);
  const std::string continuation = std::string(DisplayWidth(prefix) + 1, ' ');

  if (quality_lines.empty()) {
    std::cout << prefix << ' ' << PadRight(std::string(), kQualityColumnWidth)
              << ' ' << text << '\n';
    return;
  }
  for (std::size_t index = 0; index < quality_lines.size(); ++index) {
    std::cout << (index == 0 ? prefix : continuation) << ' '
              << PadRight(
                     ClipDisplay(quality_lines[index], kQualityColumnWidth),
                     kQualityColumnWidth);
    if (index + 1 == quality_lines.size()) {
      std::cout << ' ' << text;
    }
    std::cout << '\n';
  }
}

void WriteMessageCsv(std::ostream& output, const MessageRow& row) {
  std::string alternatives_text;
  for (const AlternativeVideo& alternative : row.alternative_videos) {
    if (!alternatives_text.empty()) {
      alternatives_text += "; ";
    }
    alternatives_text += std::to_string(alternative.height) + "p " +
                         FormatSizeShort(alternative.size);
    if (!alternative.codec.empty()) {
      alternatives_text += " " + alternative.codec;
    }
  }
  output << row.message_id << ',' << CsvEscape(FormatTimestamp(row.date)) << ','
         << CsvEscape(row.type) << ',' << row.file_id << ','
         << CsvEscape(row.file_name) << ',' << CsvEscape(row.mime_type) << ','
         << row.duration << ',' << row.width << ',' << row.height << ','
         << CsvEscape(FormatSizeShort(row.size)) << ','
         << (row.supports_streaming ? "true" : "false") << ','
         << (row.has_stickers ? "true" : "false") << ','
         << CsvEscape(alternatives_text) << ',' << CsvEscape(row.text) << '\n';
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
