# Reference https://github.com/facebook/zstd/blob/v1.4.3/lib/BUCK

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "zstd",
    srcs = glob([
        "common/*.h",
        "compress/*.h",
        "decompress/*.h",
        "deprecated/*.h",
        "dictBuilder/*.h",
        "legacy/*.h",
        "common/*.c",
        "compress/*.c",
        "decompress/*.c",
        "deprecated/*.c",
        "dictBuilder/*.c",
        "legacy/*.c",
    ]),
    hdrs = ["zstd.h"],
    defines = [
        "ZSTD_LEGACY_SUPPORT=4",
        "ZSTD_MULTITHREAD",
    ],
    includes = [
        ".",
        "common",
        "compress",
        "decompress",
        "deprecated",
        "dictBuilder",
        "legacy",
    ],
    local_defines = [
        "XXH_NAMESPACE=ZSTD_",
    ],
    visibility = ["//visibility:public"],
)
