#ifndef CODELAB_PB_TO_ARROW_STATUS_UTIL_H_
#define CODELAB_PB_TO_ARROW_STATUS_UTIL_H_

#include "absl/status/status.h"
#include "arrow/result.h"
#include "arrow/status.h"
#include "gtl/compiler_specific.h"
#include "status/status_or.h"
#include "status/status_util.h"

namespace hcoona {
namespace codelab {

absl::Status FromArrowStatus(const arrow::Status& arrow_status);

template <typename T>
StatusOr<T> FromArrowResult(const arrow::Result<T>& arrow_result);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_ARROW_STATUS_UTIL_H_
