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

#include "one/codelab/minikafka/protocol/request_header_data.h"

#include <limits>
#include <vector>

#include "one/base/macros.h"
#include "one/codelab/minikafka/base/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

absl::Status RequestHeaderData::ParseFrom(KafkaBinaryReader* reader,
                                          int16_t header_version) {
  ONE_RETURN_IF_NOT_OK(reader->ReadInt16(&api_key_));
  ONE_RETURN_IF_NOT_OK(reader->ReadInt16(&api_version_));
  ONE_RETURN_IF_NOT_OK(reader->ReadInt32(&correlation_id_));
  if (header_version >= 1) {
    int16_t client_id_length{};
    ONE_RETURN_IF_NOT_OK(reader->ReadInt16(&client_id_length));
    if (client_id_length < 0) {
      client_id_.clear();
    } else {
      ONE_RETURN_IF_NOT_OK(reader->ReadString(&client_id_, client_id_length));
    }
  } else {
    client_id_.clear();
  }

  if (header_version >= 2) {
    // TODO(zhangshuai.ustc): Store unknown tagged fields.

    std::vector<ZeroCopyRawTaggedFields> tagged_fields;
    ONE_RETURN_IF_NOT_OK(reader->ReadTaggedFields(&tagged_fields));
  }

  return absl::OkStatus();
}

}  // namespace minikafka
}  // namespace hcoona
