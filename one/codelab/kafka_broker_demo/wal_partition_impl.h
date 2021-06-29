#pragma once

#include "one/codelab/kafka_broker_demo/partition.h"

namespace hcoona {
namespace codelab {

class WalPartitionImpl : public Partition {
 public:
  void ProduceAsync(
      const ProduceRequest& request,
      std::function<void(absl::StatusOr<ProduceResponse>)> callback) override;
};

}  // namespace codelab
}  // namespace hcoona
