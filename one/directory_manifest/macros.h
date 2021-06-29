#ifndef DIRECTORY_MANIFEST_MACROS_H_
#define DIRECTORY_MANIFEST_MACROS_H_

#include "gtl/macros.h"

#define RETURN_IF_ERROR(...)              \
  do {                                    \
    absl::Status _status = (__VA_ARGS__); \
    if (!_status.ok()) return _status;    \
  } while (0)

#endif  // DIRECTORY_MANIFEST_MACROS_H_
