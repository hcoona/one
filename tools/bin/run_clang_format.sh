#!/bin/bash

set -e

pushd $(git rev-parse --show-toplevel) >/dev/null
git ls-files \
  | grep -v third_party \
  | grep -v gtl \
  | grep -v nail \
  | grep -E '\.(h|hpp|c|cc|cpp)$' \
  | xargs clang-format -i
popd # git rev-parse --show-toplevel
