#pragma once

#include <string>

#include "absl/hash/hash.h"

namespace hcoona {
namespace codelab {

class PartitionKey final {
 public:
  PartitionKey(std::string topic_name, int32_t partition_id)
      : topic_name(topic_name), partition_id(partition_id) {}

  const std::string topic_name;
  const int32_t partition_id;

  friend bool operator==(const PartitionKey& lhs, const PartitionKey& rhs);
  friend bool operator!=(const PartitionKey& lhs, const PartitionKey& rhs);
  friend bool operator<(const PartitionKey& lhs, const PartitionKey& rhs);

  template <typename H>
  friend H AbslHashValue(H h, const PartitionKey& partition_key);

  std::string DebugString() const;
};

template <typename T>
PartitionKey MakePartitionKey(const T& t) {
  return PartitionKey(t.topic_name, t.partition_id);
}

}  // namespace codelab
}  // namespace hcoona
