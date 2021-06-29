#include "one/codelab/kafka_broker_demo/streaming_columnar_partition_impl.h"

namespace hcoona {
namespace codelab {

void StreamingColumnarPartitionImpl::ProduceAsync(
    const ProduceRequest& request,
    std::function<void(absl::StatusOr<ProduceResponse>)> callback) {}

}  // namespace codelab
}  // namespace hcoona
