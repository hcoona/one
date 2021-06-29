#include "one/codelab/kafka_broker_demo/partition_key.h"

#include <utility>

#include "third_party/absl/strings/str_cat.h"

namespace hcoona {
namespace codelab {

bool operator==(const PartitionKey& lhs, const PartitionKey& rhs) {
  return lhs.topic_name == rhs.topic_name &&
         lhs.partition_id == rhs.partition_id;
}

bool operator!=(const PartitionKey& lhs, const PartitionKey& rhs) {
  return !(lhs == rhs);
}

bool operator<(const PartitionKey& lhs, const PartitionKey& rhs) {
  int result = lhs.topic_name.compare(rhs.topic_name);
  if (result < 0) {
    return true;
  } else if (result == 0) {
    return lhs.partition_id < rhs.partition_id;
  } else {
    return false;
  }
}

template <typename H>
H AbslHashValue(H h, const PartitionKey& partition_key) {
  return H::combine(std::move(h), partition_key.topic_name,
                    partition_key.partition_id);
}

std::string PartitionKey::DebugString() const {
  return absl::StrCat("Partition(", topic_name, ", ", partition_id, ")");
}

}  // namespace codelab
}  // namespace hcoona
