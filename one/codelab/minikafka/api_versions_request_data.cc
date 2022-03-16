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

#include "one/codelab/minikafka/api_versions_request_data.h"

#include <limits>

#include "one/base/macros.h"
#include "one/codelab/minikafka/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

absl::Status ApiVersionsRequestData::ParseFrom(KafkaBinaryReader* reader,
                                               int16_t api_version) {
  if (api_version >= 3) {
    uint32_t length;
    ONE_RETURN_IF_NOT_OK(reader->ReadVarint32(&length));

    if (length != 0) {
      //   return absl::UnknownError(
      //       "non-nullable field clientSoftwareName was serialized as null");
      // } else {
      length--;

      if (length > static_cast<uint32_t>(std::numeric_limits<int16_t>::max())) {
        return absl::UnknownError(absl::StrCat(
            "string field clientSoftwareName had invalid length ", length));
      }

      ONE_RETURN_IF_NOT_OK(reader->ReadString(&client_software_name_,
                                              static_cast<int32_t>(length)));
    }
  } else {
    client_software_name_.clear();
  }

  if (api_version >= 3) {
    uint32_t length;
    ONE_RETURN_IF_NOT_OK(reader->ReadVarint32(&length));

    if (length == 0) {
      return absl::UnknownError(
          "non-nullable field clientSoftwareVersion was serialized as null");
    }
    length--;

    if (length > static_cast<uint32_t>(std::numeric_limits<int16_t>::max())) {
      return absl::UnknownError(absl::StrCat(
          "string field clientSoftwareVersion had invalid length ", length));
    }

    ONE_RETURN_IF_NOT_OK(reader->ReadString(&client_software_version_,
                                            static_cast<int32_t>(length)));
  } else {
    client_software_version_.clear();
  }

  if (api_version >= 3) {
    // TODO(zhangshuai.ustc): Store unknown tagged fields.

    uint32_t tagged_fields_count;
    ONE_RETURN_IF_NOT_OK(reader->ReadVarint32(&tagged_fields_count));

    for (uint32_t i = 0; i < tagged_fields_count; i++) {
      uint32_t tag;
      ONE_RETURN_IF_NOT_OK(reader->ReadVarint32(&tag));
      uint32_t length;
      ONE_RETURN_IF_NOT_OK(reader->ReadVarint32(&length));

      if (length > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
        return absl::UnknownError(
            absl::StrCat("tagged_field length too large. length=", length));
      }

      std::string data;
      ONE_RETURN_IF_NOT_OK(
          reader->ReadString(&data, static_cast<int32_t>(length)));
    }
  }

  return absl::OkStatus();
}

}  // namespace minikafka
}  // namespace hcoona
