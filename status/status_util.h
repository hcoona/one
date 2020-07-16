#ifndef STATUS_STATUS_UTIL_H_
#define STATUS_STATUS_UTIL_H_

#include "absl/status/status.h"
#include "gtl/compiler_specific.h"

#define RETURN_STATUS_IF_NOT_OK(status) \
  do {                                  \
    ::absl::Status __s = (status);      \
    if (UNLIKELY(!__s.ok())) {          \
      return __s;                       \
    }                                   \
  } while (false)

#endif  // STATUS_STATUS_UTIL_H_
