// A demo for how a Apache Kafka Broker manage producing requests to many
// partitions with help of our Service facilities.
//
// We additionally make the demo more complicated by introducing a shadow
// partition managers(called StreamingColumnarPartitionImpl) aside the origin
// one(called WalPartitionImpl). Writing to either manager would not block the
// other one. However, the visibility of StreamingColumnarPartitionImpl must be
// less or equal to WalPartitionImpl.
//
// The producing request would firstly reach the RPC server aside Broker, then
// calling Broker::Produce() method concurrently with a continuation callback.
// We must trigger the callback no matter the producing succeeded or failed.

// The producing request batched a message set, which contains one or more
// messages. The WalPartitionImpl would write a message set in batch, while the
// StreamingColumnarPartitionImpl would write only one message. We need to
// unpack the message set for validation. So there is an opportunity to optimize
// it by writing to StreamingColumnarPartitionImpl during the validation, but
// defer the commitment after the WalPartitionImpl committed.
#pragma once

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

#include "third_party/absl/container/btree_map.h"
#include "third_party/absl/status/statusor.h"
#include "codelab/kafka_broker_demo/api.h"
#include "codelab/kafka_broker_demo/partition.h"
#include "codelab/kafka_broker_demo/partition_key.h"
#include "service/event_dispatcher.h"
#include "service/service_impl.h"

namespace hcoona {
namespace codelab {

enum class BrokerApiEventType {
  kProduce = 1,
};

std::string BrokerApiEventType_Name(BrokerApiEventType type);

std::ostream& operator<<(std::ostream& os, BrokerApiEventType type);

class BrokerImpl : public ServiceImpl {
 public:
  explicit BrokerImpl(EventDispatcher<ServiceStateChangedEvent>*
                          state_changed_event_dispatcher);

  void AssignPartition(
      const AssignPartitionRequest& request,
      std::function<void(absl::StatusOr<AssignPartitionResponse>)> callback);

  void UnassignPartition(
      const UnassignPartitionRequest& request,
      std::function<void(absl::StatusOr<UnassignPartitionResponse>)> callback);

  void ProduceAsync(
      const ProduceRequest& request,
      std::function<void(absl::StatusOr<ProduceResponse>)> callback);

 private:
  absl::btree_multimap<PartitionKey, std::unique_ptr<Partition>> partitions_;
  EventDispatcher<>* dispatcher_;
};

}  // namespace codelab
}  // namespace hcoona
