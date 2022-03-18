# Base

This directory provided some common utilities to accelerate daily development.

* `c_string_arg.h`: to accept C-style string in either `const char*` or `const std::string&` format. This shim class would avoid repeating the same business logics in two different types, or avoid construct a temporary `std::string` instance for performance reason. `std::string_view` don't work here because it's not `\0` terminated.
* `down_cast.h`: do `dynamic_cast` in debug profile for checking violations while do `static_cast` in release profile for performance reason and avoid using RTTI.
* `macros.h`: a few utilities macros to accelerate daily development.
    * `ONE_PREDICT_TRUE`, `ONE_PREDICT_FALSE`: performance annotation for branch predicating.
    * `ONE_RETURN_IF_NOT_OK`, `ONE_ASSIGN_OR_RETURN`: handy macros for error handling with `absl::Status` & `absl::StatusOr<>`.
    * `ONE_HARDENING_ASSERT`: for hardening. See [Sanitize, Fuzz, and Harden Your C++ Code](https://www.usenix.org/conference/enigma2016/conference-program/presentation/serebryany) for further details about why hardening.

# Sub-directories

* [container](container/index.md): some useful containers not provided in C++17 STL.

## Retry Manager & Rate limiter (not started yet)

Port from [resilience4j](https://resilience4j.readme.io/), and [Polly](https://github.com/App-vNext/Polly).

Rate limiter also reference [Guava RateLimiter](https://github.com/google/guava/blob/v31.1/guava/src/com/google/common/util/concurrent/RateLimiter.java).
