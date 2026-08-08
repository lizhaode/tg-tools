#include "commands/upload.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include "commands/upload_json.h"
#include "core/text_util.h"

namespace tg_tools {
namespace {

namespace td_api = td::td_api;

constexpr int kUploadParallel = 3;
constexpr std::chrono::minutes kUploadStallTimeout(10);

struct UploadFailure {
  std::filesystem::path name;
  std::string reason;
};

enum class UploadState : std::uint8_t {
  kQueued,
  kWaiting,
  kSucceeded,
  kFailed,
};

struct UploadTask {
  UploadItem item;
  UploadState state = UploadState::kQueued;
  std::int32_t sending_id = 0;
  std::int64_t temporary_message_id = 0;
  std::int32_t file_id = 0;
  int last_reported_percent = 0;
  std::int64_t last_uploaded_size = 0;
  std::chrono::steady_clock::time_point last_progress_time{};
};

struct UploadProgress {
  std::int64_t uploaded_size = 0;
  std::int64_t total_size = 0;
  bool is_uploading_completed = false;
};

struct PendingSendResult {
  bool succeeded = false;
  std::int64_t message_id = 0;
  std::string error;
};

bool IsRegularFile(const std::filesystem::path& path) {
  std::error_code error;
  return std::filesystem::is_regular_file(path, error) && !error;
}

std::int32_t ExtractUploadedFileId(const td_api::message& message) {
  if (message.content_ == nullptr ||
      message.content_->get_id() != td_api::messageVideo::ID) {
    return 0;
  }
  const auto& content =
      static_cast<const td_api::messageVideo&>(*message.content_);
  if (content.video_ == nullptr || content.video_->video_ == nullptr) {
    return 0;
  }
  return content.video_->video_->id_;
}

std::string TdErrorText(const td_api::error& error) {
  return "TDLib error " + std::to_string(error.code_) + ": " + error.message_;
}

class UploadRunner {
 public:
  UploadRunner(TelegramClient* client, std::int64_t chat_id,
               std::vector<UploadItem> items)
      : client_(client), chat_id_(chat_id) {
    tasks_.reserve(items.size());
    for (UploadItem& item : items) {
      tasks_.push_back(UploadTask{std::move(item)});
    }
  }

  bool Run(std::string* error) {
    if (client_ == nullptr) {
      return SetError(error, "内部错误：Telegram client 为空");
    }

    StartNext();
    while (active_ > 0 || next_index_ < tasks_.size()) {
      td::ClientManager::Response response = client_->Receive(1.0);
      if (response.object) {
        ProcessResponse(std::move(response));
      }
      FailStalledUploads();
    }

    std::size_t succeeded = 0;
    for (const UploadTask& task : tasks_) {
      if (task.state == UploadState::kSucceeded) {
        ++succeeded;
      }
    }
    std::cout << "上传结果：成功 " << succeeded << "，失败 " << failures_.size()
              << '\n';
    if (!failures_.empty()) {
      return SetError(error,
                      std::to_string(failures_.size()) + " upload(s) failed");
    }
    return true;
  }

 private:
  void StartNext() {
    while (!is_closed_ && active_ < kUploadParallel &&
           next_index_ < tasks_.size()) {
      const std::size_t task_index = next_index_++;
      UploadTask& task = tasks_[task_index];
      if (task.item.name.empty() || !IsRegularFile(task.item.name)) {
        RecordFailure(task_index,
                      "文件不存在或不是常规文件：" + task.item.name.string());
        continue;
      }

      task.last_progress_time = std::chrono::steady_clock::now();

      auto local_file = td_api::make_object<td_api::inputFileLocal>();
      local_file->path_ = task.item.name.string();

      auto caption_text = td_api::make_object<td_api::formattedText>();
      caption_text->text_ = task.item.caption;

      auto input_video = td_api::make_object<td_api::inputVideo>();
      input_video->video_ = std::move(local_file);
      input_video->supports_streaming_ = true;

      auto video_content = td_api::make_object<td_api::inputMessageVideo>();
      video_content->video_ = std::move(input_video);
      video_content->caption_ = std::move(caption_text);

      auto send_message = td_api::make_object<td_api::sendMessage>();
      send_message->chat_id_ = chat_id_;
      send_message->input_message_content_ = std::move(video_content);

      const std::int32_t sending_id = next_sending_id_++;
      auto send_options = td_api::make_object<td_api::messageSendOptions>(
          nullptr, false, false, false, false, 0, false, nullptr, 0, sending_id,
          false);
      send_message->options_ = std::move(send_options);

      const std::uint64_t request_id = client_->Send(std::move(send_message));
      task.state = UploadState::kWaiting;
      task.sending_id = sending_id;
      task_by_request_id_[request_id] = task_index;
      task_by_sending_id_[sending_id] = task_index;
      ++active_;
      std::cout << "开始上传：" << OneLine(task.item.name.string()) << '\n';
    }
  }

  void ProcessResponse(td::ClientManager::Response response) {
    if (response.request_id != 0) {
      ProcessRequestResponse(std::move(response));
      return;
    }

    switch (response.object->get_id()) {
      case td_api::updateAuthorizationState::ID: {
        auto update = td::move_tl_object_as<td_api::updateAuthorizationState>(
            std::move(response.object));
        if (update->authorization_state_ != nullptr &&
            update->authorization_state_->get_id() ==
                td_api::authorizationStateClosed::ID) {
          AbortUploads("TDLib 已关闭");
        }
        break;
      }
      case td_api::updateNewMessage::ID: {
        auto update = td::move_tl_object_as<td_api::updateNewMessage>(
            std::move(response.object));
        ProcessNewMessage(*update->message_);
        break;
      }
      case td_api::updateFile::ID: {
        auto update = td::move_tl_object_as<td_api::updateFile>(
            std::move(response.object));
        ProcessFileUpdate(*update->file_);
        break;
      }
      case td_api::updateMessageSendSucceeded::ID: {
        auto update = td::move_tl_object_as<td_api::updateMessageSendSucceeded>(
            std::move(response.object));
        if (update->message_->chat_id_ == chat_id_) {
          ProcessSendResult(
              update->old_message_id_,
              PendingSendResult{true, update->message_->id_, std::string()});
        }
        break;
      }
      case td_api::updateMessageSendFailed::ID: {
        auto update = td::move_tl_object_as<td_api::updateMessageSendFailed>(
            std::move(response.object));
        if (update->message_->chat_id_ == chat_id_) {
          ProcessSendResult(
              update->old_message_id_,
              PendingSendResult{false, 0, TdErrorText(*update->error_)});
        }
        break;
      }
      case td_api::updateDeleteMessages::ID: {
        auto update = td::move_tl_object_as<td_api::updateDeleteMessages>(
            std::move(response.object));
        if (update->chat_id_ == chat_id_) {
          for (const std::int64_t message_id : update->message_ids_) {
            ProcessSendResult(
                message_id,
                PendingSendResult{false, 0, "TDLib 删除了尚未发送的消息"});
          }
        }
        break;
      }
      default:
        break;
    }
  }

  void ProcessRequestResponse(td::ClientManager::Response response) {
    const auto request = task_by_request_id_.find(response.request_id);
    if (request == task_by_request_id_.end()) {
      return;
    }

    const std::size_t task_index = request->second;
    task_by_request_id_.erase(request);
    if (tasks_[task_index].state == UploadState::kSucceeded ||
        tasks_[task_index].state == UploadState::kFailed) {
      return;
    }
    if (response.object->get_id() == td_api::error::ID) {
      const auto& td_error =
          static_cast<const td_api::error&>(*response.object);
      FailActiveTask(task_index, TdErrorText(td_error));
      return;
    }
    if (response.object->get_id() != td_api::message::ID) {
      FailActiveTask(task_index, "TDLib 返回了意外的上传响应");
      return;
    }

    auto message =
        td::move_tl_object_as<td_api::message>(std::move(response.object));
    HandleSentMessage(task_index, *message);
  }

  void ProcessNewMessage(const td_api::message& message) {
    if (message.chat_id_ != chat_id_ || message.sending_state_ == nullptr ||
        message.sending_state_->get_id() !=
            td_api::messageSendingStatePending::ID) {
      return;
    }
    const auto& state = static_cast<const td_api::messageSendingStatePending&>(
        *message.sending_state_);
    const auto task = task_by_sending_id_.find(state.sending_id_);
    if (task == task_by_sending_id_.end()) {
      return;
    }
    HandleSentMessage(task->second, message);
  }

  void HandleSentMessage(std::size_t task_index,
                         const td_api::message& message) {
    UploadTask& task = tasks_[task_index];
    if (task.state == UploadState::kSucceeded ||
        task.state == UploadState::kFailed) {
      return;
    }
    task_by_sending_id_.erase(task.sending_id);
    task.last_progress_time = std::chrono::steady_clock::now();
    if (task.temporary_message_id == 0) {
      task.temporary_message_id = message.id_;
      task_by_temporary_message_id_[message.id_] = task_index;
    } else if (task.temporary_message_id != message.id_) {
      FailActiveTask(task_index, "TDLib 返回了不一致的临时消息 ID");
      return;
    }

    if (message.sending_state_ == nullptr) {
      CompleteTask(task_index, message.id_);
      return;
    }
    if (message.sending_state_->get_id() ==
        td_api::messageSendingStateFailed::ID) {
      const auto& state = static_cast<const td_api::messageSendingStateFailed&>(
          *message.sending_state_);
      FailActiveTask(task_index, TdErrorText(*state.error_));
      return;
    }
    if (message.sending_state_->get_id() !=
        td_api::messageSendingStatePending::ID) {
      FailActiveTask(task_index, "TDLib 返回了未知的消息发送状态");
      return;
    }

    const std::int32_t file_id = ExtractUploadedFileId(message);
    if (file_id != 0 && task.file_id != file_id) {
      task.file_id = file_id;
      task_indices_by_file_id_[file_id] = task_index;
      const auto progress = latest_progress_by_file_id_.find(file_id);
      if (progress != latest_progress_by_file_id_.end()) {
        ReportProgress(task_index, progress->second);
      }
    }
    const auto pending =
        pending_results_by_temporary_message_id_.find(message.id_);
    if (pending != pending_results_by_temporary_message_id_.end()) {
      const PendingSendResult result = std::move(pending->second);
      pending_results_by_temporary_message_id_.erase(pending);
      ApplySendResult(task_index, result);
      return;
    }
  }

  void ProcessFileUpdate(const td_api::file& file) {
    UploadProgress progress;
    progress.uploaded_size =
        file.remote_ == nullptr ? 0 : file.remote_->uploaded_size_;
    progress.total_size = file.size_ > 0 ? file.size_ : file.expected_size_;
    progress.is_uploading_completed =
        file.remote_ != nullptr && file.remote_->is_uploading_completed_;
    latest_progress_by_file_id_[file.id_] = progress;

    const auto task = task_indices_by_file_id_.find(file.id_);
    if (task == task_indices_by_file_id_.end()) {
      return;
    }
    ReportProgress(task->second, progress);
  }

  void ReportProgress(std::size_t task_index, const UploadProgress& progress) {
    UploadTask& task = tasks_[task_index];
    if (task.state != UploadState::kWaiting || progress.total_size <= 0) {
      return;
    }

    const std::int64_t uploaded_size =
        std::min(progress.uploaded_size, progress.total_size);
    if (uploaded_size > task.last_uploaded_size) {
      task.last_uploaded_size = uploaded_size;
      task.last_progress_time = std::chrono::steady_clock::now();
    }
    int percent = static_cast<int>(uploaded_size * 100 / progress.total_size);
    if (progress.is_uploading_completed) {
      percent = 100;
    }
    if (percent < 100 && percent < task.last_reported_percent + 5) {
      return;
    }
    if (percent <= task.last_reported_percent) {
      return;
    }

    task.last_reported_percent = percent;
    std::cout << "上传进度：" << OneLine(task.item.name.string()) << ' '
              << percent << "% (" << FormatSize(uploaded_size) << "/"
              << FormatSize(progress.total_size) << ")\n";
  }

  void ProcessSendResult(std::int64_t temporary_message_id,
                         PendingSendResult result) {
    const auto task = task_by_temporary_message_id_.find(temporary_message_id);
    if (task == task_by_temporary_message_id_.end()) {
      if (!task_by_sending_id_.empty()) {
        pending_results_by_temporary_message_id_[temporary_message_id] =
            std::move(result);
      }
      return;
    }
    ApplySendResult(task->second, result);
  }

  void AbortUploads(const std::string& reason) {
    if (is_closed_) {
      return;
    }
    is_closed_ = true;
    for (std::size_t index = 0; index < tasks_.size(); ++index) {
      const UploadState state = tasks_[index].state;
      if (state == UploadState::kQueued || state == UploadState::kWaiting) {
        RecordFailure(index, reason);
      }
    }
    active_ = 0;
    next_index_ = tasks_.size();
    task_by_request_id_.clear();
    task_by_sending_id_.clear();
    task_by_temporary_message_id_.clear();
    task_indices_by_file_id_.clear();
    latest_progress_by_file_id_.clear();
    pending_results_by_temporary_message_id_.clear();
  }

  void ApplySendResult(std::size_t task_index,
                       const PendingSendResult& result) {
    if (result.succeeded) {
      CompleteTask(task_index, result.message_id);
      return;
    }
    FailActiveTask(task_index, result.error);
  }

  void CompleteTask(std::size_t task_index, std::int64_t message_id) {
    UploadTask& task = tasks_[task_index];
    if (task.state == UploadState::kSucceeded ||
        task.state == UploadState::kFailed) {
      return;
    }
    task.state = UploadState::kSucceeded;
    task_by_sending_id_.erase(task.sending_id);
    task_by_temporary_message_id_.erase(task.temporary_message_id);
    --active_;
    std::cout << "上传完成：" << OneLine(task.item.name.string())
              << " message_id=" << message_id << '\n';
    StartNext();
  }

  void FailActiveTask(std::size_t task_index, const std::string& reason) {
    const UploadState state = tasks_[task_index].state;
    if (state != UploadState::kWaiting) {
      return;
    }
    RecordFailure(task_index, reason);
    --active_;
    StartNext();
  }

  void FailStalledUploads() {
    const auto now = std::chrono::steady_clock::now();
    for (std::size_t index = 0; index < tasks_.size(); ++index) {
      UploadTask& task = tasks_[index];
      if (task.state != UploadState::kWaiting) {
        continue;
      }
      if (now - task.last_progress_time < kUploadStallTimeout) {
        continue;
      }
      FailActiveTask(index, "上传停滞超过 " +
                                std::to_string(kUploadStallTimeout.count()) +
                                " 分钟");
    }
  }

  void RecordFailure(std::size_t task_index, const std::string& reason) {
    UploadTask& task = tasks_[task_index];
    if (task.state == UploadState::kSucceeded ||
        task.state == UploadState::kFailed) {
      return;
    }
    task.state = UploadState::kFailed;
    task_by_sending_id_.erase(task.sending_id);
    task_by_temporary_message_id_.erase(task.temporary_message_id);
    failures_.push_back(UploadFailure{task.item.name, reason});
    std::cerr << "上传失败：" << OneLine(task.item.name.string())
              << " reason=" << OneLine(reason) << '\n';
  }

  TelegramClient* client_ = nullptr;
  std::int64_t chat_id_ = 0;
  std::vector<UploadTask> tasks_;
  std::map<std::uint64_t, std::size_t> task_by_request_id_;
  std::map<std::int32_t, std::size_t> task_by_sending_id_;
  std::map<std::int64_t, std::size_t> task_by_temporary_message_id_;
  std::map<std::int32_t, std::size_t> task_indices_by_file_id_;
  std::map<std::int32_t, UploadProgress> latest_progress_by_file_id_;
  std::map<std::int64_t, PendingSendResult>
      pending_results_by_temporary_message_id_;
  std::size_t next_index_ = 0;
  std::int32_t next_sending_id_ = 1;
  int active_ = 0;
  bool is_closed_ = false;
  std::vector<UploadFailure> failures_;
};

bool ValidateNoDuplicatePaths(const std::vector<UploadItem>& items,
                              std::string* error) {
  std::set<std::filesystem::path> seen_paths;
  for (const UploadItem& item : items) {
    if (item.name.empty()) {
      continue;
    }
    std::error_code normalize_error;
    std::filesystem::path normalized =
        std::filesystem::weakly_canonical(item.name, normalize_error);
    if (normalize_error) {
      normalized = std::filesystem::absolute(item.name, normalize_error);
    }
    if (!seen_paths.insert(normalized).second) {
      return SetError(error, "上传列表中存在重复文件：" + item.name.string());
    }
  }
  return true;
}

}  // namespace

bool RunUploadCommand(TelegramClient* client, const ParsedArgs& args,
                      std::string* error) {
  std::int64_t chat_id = 0;
  if (!ParseInt64Option(args, "chat", &chat_id, error)) {
    return false;
  }
  const std::string json_path = ParseStringOption(args, "json", "");
  const bool has_json = args.options.find("json") != args.options.end();
  const bool has_file = args.options.find("file") != args.options.end();

  if (has_json && has_file) {
    return SetError(error, "--file 和 --json 只能二选一");
  }
  if (!has_json && !has_file) {
    return SetError(error, "必须指定 --file 或 --json 中的一个");
  }
  if (has_json && json_path.empty()) {
    return SetError(error, "参数 --json 需要一个文件路径");
  }

  std::vector<UploadItem> items;
  if (has_json) {
    if (!ParseUploadJsonFile(json_path, &items, error)) {
      return false;
    }
  } else {
    items.push_back(UploadItem{ParseStringOption(args, "file", ""),
                               ParseStringOption(args, "caption", "")});
  }
  if (!ValidateNoDuplicatePaths(items, error)) {
    return false;
  }
  UploadRunner runner(client, chat_id, std::move(items));
  return runner.Run(error);
}

}  // namespace tg_tools
