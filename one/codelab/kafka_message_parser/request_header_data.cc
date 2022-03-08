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

#include "one/codelab/kafka_message_parser/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

absl::Status RequestHeaderData::ParseFrom(
    absl::Span<const uint8_t> message_bytes, int16_t version) {
  KafkaBinaryReader reader(message_bytes);

  return absl::UnimplementedError("Not implemented yet.");
}

}  // namespace minikafka
}  // namespace hcoona
