#ifndef CODELAB_PB_TO_ARROW_STATUS_UTIL_H_
#define CODELAB_PB_TO_ARROW_STATUS_UTIL_H_

#include "third_party/absl/status/status.h"
#include "third_party/absl/status/statusor.h"
#include "third_party/arrow/src/arrow/result.h"
#include "third_party/arrow/src/arrow/status.h"

namespace hcoona {
namespace codelab {

absl::Status FromArrowStatus(const arrow::Status& arrow_status);

template <typename T>
absl::StatusOr<T> FromArrowResult(const arrow::Result<T>& arrow_result) {
  if (arrow_result.ok()) {
    return absl::StatusOr<T>(arrow_result.ValueUnsafe());
  } else {
    return absl::StatusOr<T>(FromArrowStatus(arrow_result.status()));
  }
}

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_ARROW_STATUS_UTIL_H_
