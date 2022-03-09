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

#include "one/codelab/kafka_message_parser/request_header_data.h"

#include "one/base/macros.h"
#include "one/codelab/kafka_message_parser/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

absl::Status RequestHeaderData::ParseFrom(
    absl::Span<const uint8_t> message_bytes, int16_t version) {
  KafkaBinaryReader reader(message_bytes);

  ONE_RETURN_IF_NOT_OK(reader.Read(&request_api_key_));
  ONE_RETURN_IF_NOT_OK(reader.Read(&request_api_version_));
  ONE_RETURN_IF_NOT_OK(reader.Read(&correlation_id_));
  if (version >= 1) {
    int16_t client_id_length{};
    ONE_RETURN_IF_NOT_OK(reader.Read(&client_id_length));
    if (client_id_length < 0) {
      client_id_.clear();
    } else {
      ONE_RETURN_IF_NOT_OK(reader.ReadString(&client_id_, client_id_length));
    }
  } else {
    client_id_.clear();
  }

  if (version >= 2) {
    return absl::UnimplementedError(
        "Unknown tagged fields are not supported yet.");
  }

  return absl::OkStatus();
}

}  // namespace minikafka
}  // namespace hcoona
