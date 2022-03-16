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
#include <vector>

#include "one/base/macros.h"
#include "one/codelab/minikafka/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

absl::Status ApiVersionsRequestData::ParseFrom(KafkaBinaryReader* reader,
                                               int16_t api_version) {
  if (api_version >= 3) {
    ONE_RETURN_IF_NOT_OK(reader->ReadCompactString(&client_software_name_));
  } else {
    client_software_name_.clear();
  }

  if (api_version >= 3) {
    ONE_RETURN_IF_NOT_OK(reader->ReadCompactString(&client_software_version_));
  } else {
    client_software_version_.clear();
  }

  if (api_version >= 3) {
    // TODO(zhangshuai.ustc): Store unknown tagged fields.

    std::vector<ZeroCopyRawTaggedFields> tagged_fields;
    ONE_RETURN_IF_NOT_OK(reader->ReadTaggedFields(&tagged_fields));
  }

  return absl::OkStatus();
}

}  // namespace minikafka
}  // namespace hcoona
