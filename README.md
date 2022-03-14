# ONE: the mono repo for me

See https://monorepo.tools for "What is a monorepo" & "Why a monorepo". And [Why Google Stores Billions of Lines of Code in a Single Repository](https://cacm.acm.org/magazines/2016/7/204032-why-google-stores-billions-of-lines-of-code-in-a-single-repository/fulltext) is also a good reading material.

This branch is built from the bottom to apply licenses. I'll migrate codes from the master branch in the future time.

## Pre-requests

The project only compiles on Linux with LLVM 13 toolchain. There was too few time to make it works on other platform.

1. A linux OS with kernel version 4.14 or higher version. (Only verified on Debian & Ubuntu distro)
1. LLVM 13 toolchain with libc++abi and libfuzzer (for fuzz testing).
1. JDK 11 or higher version
1. [bazelisk](https://github.com/bazelbuild/bazelisk) v1.11.0 or higher version.

## Development

### Generate `clang.bazelrc`

```bash
tools/setup_clang.sh <your LLVM prefix directory>
```

### Compile

```bash
bazel build --config=libc++ //...
```

### Unit Test

```bash
# clang-asan profile is broken now, use asan profile instead.
bazel test --config=clang-asan //... --test_tag_filters=-benchmark
```

### Benchmark

```bash
bazel test -c opt //... --test_tag_filters=benchmark
```
