load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "rapidjson",
    hdrs = glob(["include/rapidjson/**/*.h"]),
    defines = [
        "RAPIDJSON_HAS_STDSTRING",
        "RAPIDJSON_SSE2",
        "RAPIDJSON_SSE42",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
