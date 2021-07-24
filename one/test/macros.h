#pragma once

#define ONE_ASSERT_STATUS_OK(status) \
  do {                               \
    const auto& __s = (status);      \
    ASSERT_TRUE(__s.ok()) << __s;    \
  } while (false)