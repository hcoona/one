#pragma once

#include <cstddef>
#include "gsl/gsl"

namespace codelab {

ssize_t Encode(uint32_t value, gsl::span<gsl::byte> buffer);

}  // namespace codelab
