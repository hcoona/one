package(default_visibility = ["//visibility:public"])

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_grpc_library(
    name = "hello_cc_proto",
    srcs = ["hello.proto"],
    generate_mocks = True,
    proto_only = False,
    well_known_protos = False,
    deps = [],
)
