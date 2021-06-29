#pragma once

#define ONE_RETURN_STATUS_IF_NOT_OK(status) \
  do {                                      \
    const auto& __s = (status);             \
    if (!__s.ok()) {                        \
      return __s;                           \
    }                                       \
  } while (false)

#define ONE_CHECK_STATUS_OK(status) \
  do {                              \
    const auto& __s = (status);     \
    CHECK(__s.ok()) << __s;         \
  } while (false)
