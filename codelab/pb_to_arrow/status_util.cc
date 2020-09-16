#include "codelab/pb_to_arrow/status_util.h"

namespace hcoona {
namespace codelab {

absl::Status FromArrowStatus(const arrow::Status& arrow_status) {
  // TODO(zhangshuai.ustc): Attach arrow::Status as payload.
  // Ordered by StatusCode in arrow/status.h
  if (arrow_status.ok()) {
    return absl::OkStatus();
  } else if (arrow_status.IsKeyError()) {
    return absl::NotFoundError(arrow_status.message());
  } else if (arrow_status.IsTypeError() || arrow_status.IsInvalid()) {
    return absl::InvalidArgumentError(arrow_status.message());
  } else if (arrow_status.IsIndexError()) {
    return absl::OutOfRangeError(arrow_status.message());
  } else if (arrow_status.IsUnknownError()) {
    return absl::UnknownError(arrow_status.message());
  } else if (arrow_status.IsNotImplemented()) {
    return absl::UnimplementedError(arrow_status.message());
  } else if (arrow_status.IsAlreadyExists()) {
    return absl::AlreadyExistsError(arrow_status.message());
  } else {
    return absl::InternalError(arrow_status.message());
  }
}

}  // namespace codelab
}  // namespace hcoona
