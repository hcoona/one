load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

OT_DEFINES = [
    "HAVE_ABSEIL=1",
    # "ENABLE_METRICS_PREVIEW=1",
    # "ENABLE_LOGS_PREVIEW=1",
]

OT_TEST_FLAGS = [
    "-Wno-dangling-else",
    "-Wno-defaulted-function-deleted",
    "-Wno-dynamic-class-memaccess",
    "-Wno-pessimizing-move",
]

cc_library(
    name = "api",
    hdrs = glob(
        [
            "api/include/opentelemetry/**/*.h",
        ],
    ),
    defines = OT_DEFINES,
    strip_include_prefix = "api/include",
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/types:any",
    ],
)

cc_test(
    name = "api_aggregated_tests",
    size = "small",
    srcs = glob(
        [
            "api/test/**/*.cc",
            "api/test/**/*.h",
        ],
        exclude = [
            "api/test/**/*_benchmark.cc",
        ],
    ),
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    deps = [
        ":api",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "baggage_benchmark",
    size = "small",
    srcs = [
        "api/test/baggage/baggage_benchmark.cc",
    ],
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":api",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "spinlock_benchmark",
    size = "small",
    srcs = [
        "api/test/common/spinlock_benchmark.cc",
    ],
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":api",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "span_benchmark",
    size = "small",
    srcs = [
        "api/test/trace/span_benchmark.cc",
    ],
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":api",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "span_id_benchmark",
    size = "small",
    srcs = [
        "api/test/trace/span_id_benchmark.cc",
    ],
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":api",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "sdk",
    srcs = glob(
        [
            "sdk/src/**/*.cc",
            "sdk/src/**/*.h",
        ],
        exclude = [
            "sdk/src/**/*_windows.cc",
        ],
    ),
    hdrs = glob(
        [
            "sdk/include/opentelemetry/**/*.h",
        ],
    ),
    copts = [
        "-iquote external/io_opentelemetry_cpp/sdk",
    ],
    defines = OT_DEFINES,
    strip_include_prefix = "sdk/include",
    visibility = ["//visibility:public"],
    deps = [
        ":api",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_library(
    name = "memory_exporter",
    hdrs = glob(
        [
            "exporters/memory/include/opentelemetry/**/*.h",
        ],
    ),
    defines = OT_DEFINES,
    strip_include_prefix = "exporters/memory/include",
    visibility = ["//visibility:public"],
    deps = [
        ":sdk",
    ],
)

cc_library(
    name = "ostream_exporter",
    srcs = glob(
        [
            "exporters/ostream/src/**/*.cc",
            "exporters/ostream/src/**/*.h",
        ],
    ),
    hdrs = glob(
        [
            "exporters/ostream/include/opentelemetry/**/*.h",
        ],
    ),
    defines = OT_DEFINES,
    strip_include_prefix = "exporters/ostream/include",
    visibility = ["//visibility:public"],
    deps = [
        ":sdk",
    ],
)

cc_test(
    name = "sdk_aggregated_tests",
    size = "small",
    srcs = glob(
        [
            "sdk/test/**/*.cc",
            "sdk/test/**/*.h",
        ],
        exclude = [
            "sdk/test/**/*_benchmark.cc",
        ],
    ),
    copts = [
        "-iquote external/io_opentelemetry_cpp/sdk",
    ] + OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    deps = [
        ":memory_exporter",
        ":sdk",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "circular_buffer_benchmark",
    size = "medium",
    srcs = [
        "sdk/test/common/baseline_circular_buffer.h",
        "sdk/test/common/circular_buffer_benchmark.cc",
    ],
    copts = [
        "-iquote external/io_opentelemetry_cpp/sdk",
    ] + OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":sdk",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "random_benchmark",
    size = "small",
    srcs = [
        "sdk/test/common/random_benchmark.cc",
    ],
    copts = [
        "-iquote external/io_opentelemetry_cpp/sdk",
    ] + OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":sdk",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "sampler_benchmark",
    size = "small",
    srcs = [
        "sdk/test/trace/sampler_benchmark.cc",
    ],
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    tags = [
        "benchmark",
    ],
    deps = [
        ":memory_exporter",
        ":sdk",
        "@com_github_google_benchmark//:benchmark",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "memory_exporter_tests",
    size = "small",
    srcs = glob(
        [
            "exporters/memory/test/**/*.cc",
            "exporters/memory/test/**/*.h",
        ],
    ),
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    deps = [
        ":memory_exporter",
        ":sdk",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "ostream_exporter_tests",
    size = "small",
    srcs = glob(
        [
            "exporters/ostream/test/**/*.cc",
            "exporters/ostream/test/**/*.h",
        ],
    ),
    copts = OT_TEST_FLAGS,
    linkopts = [
        "-ldl",
    ],
    deps = [
        ":ostream_exporter",
        ":sdk",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
