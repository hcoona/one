#include "codelab/kafka_broker_demo/broker_impl.h"

namespace hcoona {
namespace codelab {

BrokerImpl::BrokerImpl(
    EventDispatcher<ServiceStateChangedEvent>* state_changed_event_dispatcher)
    : ServiceImpl("Broker", state_changed_event_dispatcher) {}

void BrokerImpl::AssignPartition(
    const AssignPartitionRequest& request,
    std::function<void(absl::StatusOr<AssignPartitionResponse>)> callback) {
  auto partition_key = MakePartitionKey(request);

  if (partitions_.contains(partition_key)) {
    callback(absl::AlreadyExistsError("Partition already exists."));
    return;
  }
}

void BrokerImpl::UnassignPartition(
    const UnassignPartitionRequest& request,
    std::function<void(absl::StatusOr<UnassignPartitionResponse>)> callback) {
  //
}

void BrokerImpl::ProduceAsync(
    const ProduceRequest& request,
    std::function<void(absl::StatusOr<ProduceResponse>)> callback) {
  auto partition_key = MakePartitionKey(request);

  int counter = 0;
  for (auto it = partitions_.lower_bound(partition_key);
       it != partitions_.end() && it->first == partition_key; it++) {
    counter++;
    // TODO(zhangshuai.ustc): Ensure callback with shared_ptr? Or process it
    // with event notification?
    it->second->ProduceAsync(request, callback);
  }
  if (counter == 0) {
    callback(absl::NotFoundError("Cannot found partition"));
  }
}

}  // namespace codelab
}  // namespace hcoona
