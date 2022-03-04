# ONE: the mono repo for me

See https://monorepo.tools for "What is a monorepo" & "Why a monorepo". And [Why Google Stores Billions of Lines of Code in a Single Repository](https://cacm.acm.org/magazines/2016/7/204032-why-google-stores-billions-of-lines-of-code-in-a-single-repository/fulltext) is also a good reading material.

## Pre-requests

The project only compiles on Linux with LLVM 13 toolchain. There was too few time to make it works on other platform.

1. A linux OS with kernel version 4.14 or higher version. (Only verified on Debian & Ubuntu distro)
1. LLVM 13 toolchain with libc++abi and libfuzzer (for fuzz testing).
1. JDK 11 or higher version
1. [bazelisk](https://github.com/bazelbuild/bazelisk) v1.11.0 or higher version.

## Development

### Compile

```bash
bazel build //...
```

### Unit Test

```bash
bazel test //... --test_tag_filters=-benchmark
```

### Benchmark

```bash
bazel test -c opt //... --test_tag_filters=benchmark
```
