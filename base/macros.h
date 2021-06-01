#pragma once

#define RETURN_STATUS_IF_NOT_OK(status) \
  do {                                  \
    const auto& __s = (status);         \
    if (!__s.ok()) {                    \
      return __s;                       \
    }                                   \
  } while (false)

#define CHECK_STATUS_OK(status) \
  do {                          \
    const auto& __s = (status); \
    CHECK(__s.ok()) << __s;     \
  } while (false)
