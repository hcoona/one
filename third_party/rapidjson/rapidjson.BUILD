load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "rapidjson",
    hdrs = glob(["include/rapidjson/**/*.h"]),
    defines = [
        "RAPIDJSON_HAS_STDSTRING",
    ] + select({
        "@//config:enable_sse2": ["RAPIDJSON_SSE2"],
        "@//config:enable_sse42": ["RAPIDJSON_SSE2"],
        "//conditions:default": [],
    }) + select({
        "@//config:enable_sse42": ["RAPIDJSON_SSE42"],
        "//conditions:default": [],
    }),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
