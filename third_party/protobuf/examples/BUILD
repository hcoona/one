# This BUILD file shows how to use protobuf with bazel. Before you can use
# proto_library/<lang>_proto_library rules in a BUILD file, you need to
# include protobuf repo as remote repositories in your WORKSPACE file. See
# the WORKSPACE file in the same directory with this BUILD file for an
# example.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

# For each .proto file, a proto_library target should be defined. This target
# is not bound to any particular language. Instead, it defines the dependency
# graph of the .proto files (i.e., proto imports) and serves as the provider
# of .proto source files to the protocol compiler.
#
# Remote repository "com_google_protobuf" must be defined to use this rule.
proto_library(
    name = "addressbook_proto",
    srcs = ["addressbook.proto"],
    deps = ["//third_party/protobuf:timestamp_proto"],
)

# The cc_proto_library rule generates C++ code for a proto_library rule. It
# must have exactly one proto_library dependency. If you want to use multiple
# proto_library targets, create a separate cc_proto_library target for each
# of them.
#
# Remote repository "com_google_protobuf_cc" must be defined to use this rule.
cc_proto_library(
    name = "addressbook_cc_proto",
    deps = [":addressbook_proto"],
)

# cc_library/cc_binary targets can depend on cc_proto_library targets.
cc_binary(
    name = "add_person_cpp",
    srcs = ["add_person.cc"],
    deps = [":addressbook_cc_proto"],
    includes = ["."],
)

cc_binary(
    name = "list_people_cpp",
    srcs = ["list_people.cc"],
    deps = [":addressbook_cc_proto"],
    includes = ["."],
)
