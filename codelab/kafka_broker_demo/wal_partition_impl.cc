#include "codelab/kafka_broker_demo/wal_partition_impl.h"

namespace hcoona {
namespace codelab {

void WalPartitionImpl::ProduceAsync(
    const ProduceRequest& request,
    std::function<void(absl::StatusOr<ProduceResponse>)> callback) {}

}  // namespace codelab
}  // namespace hcoona
