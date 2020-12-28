#pragma once

#include <functional>

#include "absl/status/statusor.h"
#include "codelab/kafka_broker_demo/api.h"
#include "codelab/kafka_broker_demo/partition_key.h"
#include "service/service_impl.h"

namespace hcoona {
namespace codelab {

class Partition : public ServiceImpl {
 public:
  Partition(PartitionKey partiton_key,
            EventDispatcher<ServiceStateChangedEvent>*
                state_changed_event_dispatcher);

  virtual void ProduceAsync(
      const ProduceRequest& request,
      std::function<void(absl::StatusOr<ProduceResponse>)> callback) = 0;
};

}  // namespace codelab
}  // namespace hcoona
