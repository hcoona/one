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

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "glog/logging.h"
#include "one/codelab/minikafka/base/kafka_binary_writer.h"

namespace hcoona {
namespace minikafka {

class ApiVersionsResponseData {
  static constexpr int16_t kLowestSupportedVersion = 0;
  static constexpr int16_t kHighestSupportedVersion = 3;

 public:
  struct ApiVersionEntry {
    int16_t api_key;
    int16_t min_version;
    int16_t max_version;
  };
  struct SupportedFeatureKeyEntry {
    std::string name;
    int16_t min_version;
    int16_t max_version;
  };
  struct FinalizedFeatureKeyEntry {
    std::string name;
    int16_t min_version_level;
    int16_t max_version_level;
  };

  [[nodiscard]] static int16_t lowest_supported_version() {
    return kLowestSupportedVersion;
  }

  [[nodiscard]] static int16_t highest_supported_version() {
    return kHighestSupportedVersion;
  }

  [[nodiscard]] int16_t error_code() const { return error_code_; }
  void set_error_code(int16_t error_code) { error_code_ = error_code; }

  [[nodiscard]] const std::vector<ApiVersionEntry>& api_keys() const {
    return api_keys_;
  }
  void set_api_keys(std::vector<ApiVersionEntry> api_keys) {
    api_keys_ = std::move(api_keys);
  }

  [[nodiscard]] int32_t throttle_time_ms() const { return throttle_time_ms_; }
  void set_throttle_time_ms(int32_t throttle_time_ms) {
    throttle_time_ms_ = throttle_time_ms;
  }

  [[nodiscard]] const std::vector<SupportedFeatureKeyEntry>&
  supported_features() const {
    return supported_features_;
  }
  void set_supported_features(
      std::vector<SupportedFeatureKeyEntry> supported_features) {
    supported_features_ = std::move(supported_features);
  }

  [[nodiscard]] int64_t finalized_features_epoch() const {
    return finalized_features_epoch_;
  }
  void set_finalized_features_epoch(int64_t finalized_features_epoch) {
    finalized_features_epoch_ = finalized_features_epoch;
  }

  [[nodiscard]] const std::vector<FinalizedFeatureKeyEntry>&
  finalized_features() const {
    return finalized_features_;
  }
  void set_finalized_features(
      std::vector<FinalizedFeatureKeyEntry> finalized_features) {
    finalized_features_ = std::move(finalized_features);
  }

  absl::Status WriteTo(KafkaBinaryWriter* writer, int16_t api_version) const {
    ONE_RETURN_IF_NOT_OK(writer->WriteInt16(error_code_));

    if (api_version >= 3) {
      ONE_RETURN_IF_NOT_OK(writer->WriteVarint32(api_keys_.size() + 1));
    } else {
      ONE_RETURN_IF_NOT_OK(writer->WriteInt32(api_keys_.size()));
    }
    for (const auto& api_keys_element : api_keys_) {
      ONE_RETURN_IF_NOT_OK(WriteTo(writer, api_version, api_keys_element));
    }

    if (api_version >= 1) {
      ONE_RETURN_IF_NOT_OK(writer->WriteInt32(throttle_time_ms_));
    }

    uint32_t tagged_fields_num = 0;
    if (api_version >= 3) {
      if (!supported_features_.empty()) {
        tagged_fields_num++;
      }
      if (finalized_features_epoch_ != -1) {
        tagged_fields_num++;
      }
      if (!finalized_features_.empty()) {
        tagged_fields_num++;
      }
    }

    // TODO(zhangshuai.ustc): Handle known & unknown tagged fields.
    CHECK_EQ(tagged_fields_num, 0U) << "Not implemented yet.";
    ONE_RETURN_IF_NOT_OK(writer->WriteVarint32(tagged_fields_num));

    return absl::OkStatus();
  }

 private:
  static absl::Status WriteTo(KafkaBinaryWriter* writer, int16_t api_version,
                              const ApiVersionEntry& api_version_entry) {
    ONE_RETURN_IF_NOT_OK(writer->WriteInt16(api_version_entry.api_key));
    ONE_RETURN_IF_NOT_OK(writer->WriteInt16(api_version_entry.min_version));
    ONE_RETURN_IF_NOT_OK(writer->WriteInt16(api_version_entry.max_version));

    if (api_version >= 3) {
      // Empty tagged fields.
      ONE_RETURN_IF_NOT_OK(writer->WriteVarint32(0));

      // TODO(zhangshuai.ustc): Handle unknown tagged fields.
    }

    return absl::OkStatus();
  }

  int16_t error_code_{0};
  std::vector<ApiVersionEntry> api_keys_;
  int32_t throttle_time_ms_{0};
  std::vector<SupportedFeatureKeyEntry> supported_features_;
  int64_t finalized_features_epoch_{-1};
  std::vector<FinalizedFeatureKeyEntry> finalized_features_;
};

}  // namespace minikafka
}  // namespace hcoona
