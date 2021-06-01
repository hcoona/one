#pragma once

#include <string>

namespace hcoona {
namespace codelab {

class ProduceRequest {
 public:
  std::string topic_name;
  int32_t partition_id;
  std::string message_set_bytes;
};

class ProduceResponse {};

class AssignPartitionRequest {
 public:
  std::string topic_name;
  int32_t partition_id;
};

class AssignPartitionResponse {};

class UnassignPartitionRequest {
 public:
  std::string topic_name;
  int32_t partition_id;
};

class UnassignPartitionResponse {};

}  // namespace codelab
}  // namespace hcoona
