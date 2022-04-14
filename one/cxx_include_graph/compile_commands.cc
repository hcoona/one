// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.

#include "one/cxx_include_graph/compile_commands.h"

#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "rapidjson/error/en.h"
#include "rapidjson/memorystream.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/reader.h"

namespace hcoona {

namespace {

class CompileCommandsParsingHandler
    : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>,
                                          CompileCommandsParsingHandler> {
  enum class State {
    kInitialized,
    kArrayStarted,
    kArrayEnded,
    kObjectStarted,
    kObjectEnded,
    kKeyParsed,
    kValueParsed,
  };
  inline static constexpr size_t kStateCount = 7;
  static_assert(kStateCount == static_cast<size_t>(State::kValueParsed) + 1);

  friend std::ostream& operator<<(std::ostream& os, State state) {
    switch (state) {
      case State::kInitialized:
        return os << "INITIALIZED";
      case State::kArrayStarted:
        return os << "ARRAY_STARTED";
      case State::kArrayEnded:
        return os << "ARRAY_ENDED";
      case State::kObjectStarted:
        return os << "OBJECT_STARTED";
      case State::kObjectEnded:
        return os << "OBJECT_ENDED";
      case State::kKeyParsed:
        return os << "KEY_PARSED";
      case State::kValueParsed:
        return os << "VALUE_PARSED";
      default:
        LOG(FATAL) << "Not implemented for state("
                   << static_cast<std::underlying_type<State>::type>(state)
                   << ")";
    }
  }

 public:
  explicit CompileCommandsParsingHandler(
      std::vector<CompileCommandsItem>* compile_commands)
      : compile_commands_(compile_commands) {}

  bool Default() { return false; }

  bool String(const Ch* str, rapidjson::SizeType len, bool /*copy*/) {
    CHECK_EQ(state_, State::kKeyParsed);
    SetCurrentCompileCommandsItem({str, len});
    TransitionTo(State::kValueParsed);
    return true;
  }

  bool Key(const Ch* str, rapidjson::SizeType len, bool /*copy*/) {
    CHECK(state_ == State::kObjectStarted || state_ == State::kValueParsed)
        << "Invalid precondition state. state=" << state_;
    current_key_ = {str, len};
    TransitionTo(State::kKeyParsed);
    return true;
  }

  bool StartObject() {
    CHECK(state_ == State::kArrayStarted || state_ == State::kObjectEnded)
        << "Invalid precondition state. state=" << state_;
    current_.reset();
    TransitionTo(State::kObjectStarted);
    return true;
  }

  bool EndObject(rapidjson::SizeType /*unused*/) {
    CHECK_EQ(state_, State::kValueParsed);
    CHECK(current_.has_value());

    compile_commands_->emplace_back(std::move(current_).value());
    current_.reset();
    TransitionTo(State::kObjectEnded);

    return true;
  }

  bool StartArray() {
    CHECK_EQ(state_, State::kInitialized);
    TransitionTo(State::kArrayStarted);
    return true;
  }

  bool EndArray(rapidjson::SizeType /*len*/) {
    CHECK(state_ == State::kArrayStarted || state_ == State::kObjectEnded)
        << "Invalid precondition state. state=" << state_;
    CHECK(!current_.has_value());
    return true;
  }

 private:
  void TransitionTo(State new_state);
  void SetCurrentCompileCommandsItem(std::string value) {
    if (!current_.has_value()) {
      current_.emplace();
    }
    CHECK(current_.has_value());

    if (current_key_ == "command") {
      current_->command = std::move(value);
    } else if (current_key_ == "directory") {
      current_->directory = std::move(value);
    } else if (current_key_ == "file") {
      current_->file = std::move(value);
    } else {
      LOG(FATAL) << "Unexpected key: " << current_key_;
    }
  }

  std::optional<CompileCommandsItem> current_;
  std::string current_key_;

  State state_{State::kInitialized};

  std::vector<CompileCommandsItem>* compile_commands_;
};

void CompileCommandsParsingHandler::TransitionTo(State new_state) {
  // clang-format off
  static constexpr bool kTransitionAllowTable[kStateCount][kStateCount] = {
      //                    kInitialized, kArrayStarted, kArrayEnded, kObjectStarted, kObjectEnded, kKeyParsed, kValueParsed       NOLINT
      /* kInitialized   */ {       false,          true,       false,          false,        false,      false,        false},  // NOLINT
      /* kArrayStarted  */ {       false,         false,        true,           true,        false,      false,        false},  // NOLINT
      /* kArrayEnded    */ {       false,         false,       false,          false,        false,      false,        false},  // NOLINT
      /* kObjectStarted */ {       false,         false,       false,          false,        false,       true,        false},  // NOLINT
      /* kObjectEnded   */ {       false,         false,        true,           true,        false,      false,        false},  // NOLINT
      /* kKeyParsed     */ {       false,         false,       false,          false,        false,      false,         true},  // NOLINT
      /* kValueParsed   */ {       false,         false,       false,          false,         true,       true,        false},  // NOLINT
  };
  // clang-format on
  CHECK(kTransitionAllowTable[static_cast<std::underlying_type<State>::type>(
      state_)][static_cast<std::underlying_type<State>::type>(new_state)]);
  state_ = new_state;
}

}  // namespace

absl::Status Parse(const char* compile_commands_json_content,
                   std::vector<CompileCommandsItem>* compile_commands) {
  rapidjson::GenericReader<rapidjson::UTF8<>, rapidjson::UTF8<>> reader;

  rapidjson::StringStream stream(compile_commands_json_content);
  CompileCommandsParsingHandler handler(compile_commands);
  if (!reader.Parse(stream, handler)) {
    return absl::UnknownError(
        rapidjson::GetParseError_En(reader.GetParseErrorCode()));
  }
  return absl::OkStatus();
}

}  // namespace hcoona
