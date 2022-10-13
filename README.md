# ONE: the mono repo for me

See https://monorepo.tools for "What is a monorepo" & "Why a monorepo". And [Why Google Stores Billions of Lines of Code in a Single Repository](https://cacm.acm.org/magazines/2016/7/204032-why-google-stores-billions-of-lines-of-code-in-a-single-repository/fulltext) is also a good reading material.

This branch is built from the bottom to apply licenses. I'll migrate codes from the master branch in the future time.

## Pre-requests

The project only compiles on Ubuntu with LLVM 14 toolchain. There was too few time to make it works on other platforms.

1. Ubuntu OS with kernel version 4.14 or higher version. (Only verified on Ubuntu 22.04)
1. LLVM 14 toolchain with libc++abi and libfuzzer (for fuzz testing).
1. JDK 11 or higher version
1. [bazelisk](https://github.com/bazelbuild/bazelisk) v1.11.0 or higher version.

## Development

### (Optional) Add BuildBuddy API key for Bazel remoting cache

N.B. it only works for people who are working with almost same environment as me or the CI pipeline agent if you are using my READ-ONLY API key.

If you need this feature, please make a [bazel-remote](https://github.com/buchgr/bazel-remote) service for yourself or leverage [BuildBuddy](https://www.buildbuddy.io/).

```bash
echo "build:buildbuddy --remote_header=x-buildbuddy-api-key=keZr2WKcTginehDBB1D0" >> user.bazelrc
```

### (Optional) Download distdir for fast bootstrapping

Bazel would download dependencies from internet on-the-fly. There's a prepared distdir package to avoid the extra downloading. See [Bazel Distribution files directories](https://bazel.build/run/build#distribution-directory) for further details.

1. Go to [One-distdir-Official](https://dev.azure.com/zhangshuai89/GitHub%20ADO/_build?definitionId=14)
2. Open the latest succeeded run, check the "1 published" link under "Related" column.
3. Download the artifact & put it somewhere (I quote it with `$ONE_DISTDIR` later).
4. When building the project, go with the option `--distdir $ONE_DISTDIR`.

Downloading with Azure CLI tool is harder. You need to generate a PAT from Azure Devops portal to continue.

```bash
# Enter your PAT here.
az devops login

# Query the latest succeeded run-id.
az pipelines runs list --organization "https://dev.azure.com/zhangshuai89/" --project "GitHub ADO" --pipeline-ids 14  --query "[?result=='succeeded'] | [0] | id"

# Fill the run id found in previous step.
az pipelines runs artifact download --artifact-name distdir --path distdir --run-id "<run-id>" --organization "https://dev.azure.com/zhangshuai89/" --project "GitHub ADO"
```

### Generate `clang.bazelrc`

```bash
tools/setup_clang.sh /usr/lib/llvm-14
```

### Compile

```bash
bazel build --config=libc++ --config=buildbuddy //...
```

### Unit Test

```bash
# clang-asan profile is broken now, use asan profile instead.
bazel test --config=clang-asan --config=buildbuddy //... --test_tag_filters=-benchmark
```

### Benchmark

```bash
bazel test -c opt --config=libc++ --config=buildbuddy //... --test_tag_filters=benchmark
```
