#include "status/status_or.h"

#include "absl/base/attributes.h"
#include "absl/status/status.h"
#include "glog/logging.h"

namespace hcoona {
namespace internal {

ABSL_ATTRIBUTE_NORETURN void StatusOrHelper::Crash(const absl::Status& status) {
  LOG(FATAL) << "Attempting to fetch value instead of handling error "
             << status.ToString();
}

}  // namespace internal
}  // namespace hcoona
