#include "one/codelab/kafka_broker_demo/partition.h"

namespace hcoona {
namespace codelab {

Partition::Partition(
    PartitionKey partition_key,
    EventDispatcher<ServiceStateChangedEvent>* state_changed_event_dispatcher)
    : ServiceImpl(partition_key.DebugString(), state_changed_event_dispatcher) {
}

}  // namespace codelab
}  // namespace hcoona
