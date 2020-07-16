#ifndef STATUS_STATUS_UTIL_H_
#define STATUS_STATUS_UTIL_H_

#include "absl/status/status.h"
#include "glog/logging.h"
#include "gtl/compiler_specific.h"

#define RETURN_STATUS_IF_NOT_OK(status) \
  do {                                  \
    ::absl::Status __s = (status);      \
    if (UNLIKELY(!__s.ok())) {          \
      return __s;                       \
    }                                   \
  } while (false)

#define CHECK_STATUS_OK(status)        \
  do {                                 \
    ::absl::Status __s = (status);     \
    CHECK(__s.ok()) << __s.ToString(); \
  } while (false)

#endif  // STATUS_STATUS_UTIL_H_
