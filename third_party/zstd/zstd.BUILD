cc_library(
    name = "zstd",
    srcs = glob([
        "lib/**/*.c",
        "lib/**/*.h",
    ]),
    hdrs = ["lib/zstd.h"],
    copts =
        select({
            ":windows": [],
            "//conditions:default": ["-Wno-uninitialized"],
        }),
    linkopts = [
        "-pthread",
    ],
    local_defines = [
        "ZSTD_LEGACY_SUPPORT=4",
        "ZSTD_MULTITHREAD",
        "XXH_NAMESPACE=ZSTD_",
    ],
    strip_include_prefix = "lib",
    visibility = ["//visibility:public"],
)

config_setting(
    name = "windows",
    constraint_values = [
        "@bazel_tools//platforms:windows",
    ],
)
