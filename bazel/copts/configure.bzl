# Copied from https://github.com/abseil/abseil-cpp/blob/e6b050212c859fbaf67abac76105da10ec348274/absl/copts/configure_copts.bzl

"""Global copts.
This file simply selects the correct options from the generated files.  To
change global copts, edit bazel/copts/copts.bzl
"""

load(
    "//bazel/copts:copts.bzl",
    "ONE_GCC_EXCEPTIONS_FLAGS",
    "ONE_GCC_FLAGS",
    "ONE_GCC_TEST_FLAGS",
    "ONE_LLVM_EXCEPTIONS_FLAGS",
    "ONE_LLVM_FLAGS",
    "ONE_LLVM_TEST_FLAGS",
    "ONE_MSVC_EXCEPTIONS_FLAGS",
    "ONE_MSVC_FLAGS",
    "ONE_MSVC_LINKOPTS",
    "ONE_MSVC_TEST_FLAGS",
    "ONE_RANDOM_HWAES_MSVC_X64_FLAGS",
    "ONE_RANDOM_HWAES_X64_FLAGS",
)

ONE_DEFAULT_COPTS = select({
    "//bazel:windows": ONE_MSVC_FLAGS,
    "//bazel:llvm_compiler": ONE_LLVM_FLAGS,
    "//conditions:default": ONE_GCC_FLAGS,
})

# in absence of modules (--compiler=gcc or -c opt), cc_tests leak their copts
# to their (included header) dependencies and fail to build outside
ONE_TEST_COPTS = ONE_DEFAULT_COPTS + select({
    "//bazel:windows": ONE_MSVC_TEST_FLAGS,
    "//bazel:llvm_compiler": ONE_LLVM_TEST_FLAGS,
    "//conditions:default": ONE_GCC_TEST_FLAGS,
})

ONE_EXCEPTIONS_FLAG = select({
    "//bazel:windows": ONE_MSVC_EXCEPTIONS_FLAGS,
    "//bazel:llvm_compiler": ONE_LLVM_EXCEPTIONS_FLAGS,
    "//conditions:default": ONE_GCC_EXCEPTIONS_FLAGS,
})

ONE_EXCEPTIONS_FLAG_LINKOPTS = select({
    "//conditions:default": [],
})

ONE_DEFAULT_LINKOPTS = select({
    "//bazel:windows": ONE_MSVC_LINKOPTS,
    "//conditions:default": [],
})

# ONE_RANDOM_RANDEN_COPTS blaze copts flags which are required by each
# environment to build an accelerated RandenHwAes library.
ONE_RANDOM_RANDEN_COPTS = select({
    # APPLE
    ":cpu_darwin_x86_64": ONE_RANDOM_HWAES_X64_FLAGS,
    ":cpu_darwin": ONE_RANDOM_HWAES_X64_FLAGS,
    ":cpu_x64_windows_msvc": ONE_RANDOM_HWAES_MSVC_X64_FLAGS,
    ":cpu_x64_windows": ONE_RANDOM_HWAES_MSVC_X64_FLAGS,
    ":cpu_haswell": ONE_RANDOM_HWAES_X64_FLAGS,
    ":cpu_ppc": ["-mcrypto"],

    # Supported by default or unsupported.
    "//conditions:default": [],
})

# absl_random_randen_copts_init:
#  Initialize the config targets based on cpu, os, etc. used to select
#  the required values for ONE_RANDOM_RANDEN_COPTS
def absl_random_randen_copts_init():
    """Initialize the config_settings used by ONE_RANDOM_RANDEN_COPTS."""

    # CPU configs.
    # These configs have consistent flags to enable HWAES intsructions.
    cpu_configs = [
        "ppc",
        "haswell",
        "darwin_x86_64",
        "darwin",
        "x64_windows_msvc",
        "x64_windows",
    ]
    for cpu in cpu_configs:
        native.config_setting(
            name = "cpu_%s" % cpu,
            values = {"cpu": cpu},
        )
