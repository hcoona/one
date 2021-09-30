load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "gsl",
    hdrs = [
        "include/gsl/gsl",
        "include/gsl/gsl_algorithm",
        "include/gsl/gsl_assert",
        "include/gsl/gsl_byte",
        "include/gsl/gsl_util",
        "include/gsl/multi_span",
        "include/gsl/pointers",
        "include/gsl/span",
        "include/gsl/span_ext",
        "include/gsl/string_span",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "algorithm_test",
    srcs = ["tests/algorithm_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "assertion_test",
    srcs = ["tests/assertion_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "at_test",
    srcs = ["tests/at_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "bounds_test",
    srcs = ["tests/bounds_tests.cpp"],
    copts = [
        "-Wno-deprecated-declarations",
    ],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "multi_span_test",
    srcs = ["tests/multi_span_tests.cpp"],
    copts = [
        "-Wno-deprecated-declarations",
        "-Wno-missing-braces",
    ],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "no_exception_ensure_test",
    srcs = ["tests/no_exception_ensure_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "notnull_test",
    srcs = ["tests/notnull_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "owner_test",
    srcs = ["tests/owner_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "span_compatibility_test",
    srcs = ["tests/span_compatibility_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "span_ext_test",
    srcs = ["tests/span_ext_tests.cpp"],
    copts = [
        "-Wno-deprecated-declarations",
        "-Wno-missing-braces",
    ],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "span_test",
    srcs = ["tests/span_tests.cpp"],
    copts = [
        "-Wno-missing-braces",
    ],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "strict_notnull_test",
    srcs = ["tests/strict_notnull_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "strided_span_test",
    srcs = ["tests/strided_span_tests.cpp"],
    copts = [
        "-Wno-deprecated-declarations",
    ],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "string_span_test",
    srcs = ["tests/string_span_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)

cc_test(
    name = "utils_test",
    srcs = ["tests/utils_tests.cpp"],
    deps = [
        ":gsl",
        "//third_party/googletest:gtest_main",
    ],
)
