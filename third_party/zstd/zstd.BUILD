# Reference https://github.com/facebook/zstd/blob/v1.4.3/lib/BUCK

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
        "XXH_NAMESPACE=ZSTD_",
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
    visibility = ["//visibility:public"],
)
