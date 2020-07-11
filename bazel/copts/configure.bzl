# Copied from https://github.com/abseil/abseil-cpp/blob/518f175/absl/copts/configure_copts.bzl

"""absl specific copts.
This file simply selects the correct options from the generated files.  To
change Abseil copts, edit absl/copts/copts.py
"""

load(
    "//bazel/copts:copts.bzl",
    "ONE_GCC_FLAGS",
    "ONE_GCC_TEST_FLAGS",
    "ONE_LLVM_FLAGS",
    "ONE_LLVM_TEST_FLAGS",
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
# to their (included header) dependencies and fail to build outside absl
ONE_TEST_COPTS = ONE_DEFAULT_COPTS + select({
    "//bazel:windows": ONE_MSVC_TEST_FLAGS,
    "//bazel:llvm_compiler": ONE_LLVM_TEST_FLAGS,
    "//conditions:default": ONE_GCC_TEST_FLAGS,
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
    ":cpu_k8": ONE_RANDOM_HWAES_X64_FLAGS,
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
        "k8",
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
