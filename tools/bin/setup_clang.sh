#!/bin/bash

# Port from https://github.com/envoyproxy/envoy/blob/c76f2b25f7573f49db6b1a58033c0b6ed7c0f839/bazel/setup_clang.sh

BAZELRC_FILE="${BAZELRC_FILE:-$(bazel info workspace)/clang.bazelrc}"

LLVM_PREFIX="${1:-/usr}"

if [[ ! -e "${LLVM_PREFIX}/bin/llvm-config" ]]; then
  echo "Error: cannot find llvm-config in ${LLVM_PREFIX}."
  exit 1
fi

PATH="$("${LLVM_PREFIX}"/bin/llvm-config --bindir):${PATH}"
export PATH

echo "# Generated file, do not edit. If you want to disable clang, just delete this file.
build:clang --action_env='PATH=${PATH}'
build:clang --action_env=CC=clang
build:clang --action_env=CXX=clang++
build:clang --action_env='LLVM_CONFIG=${LLVM_PREFIX}/bin/llvm-config'
build:clang --repo_env='LLVM_CONFIG=${LLVM_PREFIX}/bin/llvm-config'
build:clang --linkopt='-L$(llvm-config --libdir)'
build:clang --linkopt='-Wl,-rpath,$(llvm-config --libdir)'
" > "${BAZELRC_FILE}"
