// Header-only configuration test

#include "third_party/fmt/core.h"

#ifndef FMT_HEADER_ONLY
#  error "Not in the header-only mode."
#endif
