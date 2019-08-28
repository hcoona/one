load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

######## Skylark libraries ########

http_archive(
    name = "bazel_skylib",
    sha256 = "9245b0549e88e356cd6a25bf79f97aa19332083890b7ac6481a2affb6ada9752",
    strip_prefix = "bazel-skylib-0.9.0",
    url = "https://github.com/bazelbuild/bazel-skylib/archive/0.9.0.tar.gz",  # 2019-07-13
)

http_archive(
    name = "platforms",
    sha256 = "c0e41126bcb54b6c6d219d019b04ca7c0392e42c186b48c36d4cd5d817bf209e",
    strip_prefix = "platforms-43155b81d40765f0d13008bc77cd2cca8ba9fb2a",
    url = "https://github.com/bazelbuild/platforms/archive/43155b81d40765f0d13008bc77cd2cca8ba9fb2a.tar.gz",  # 2019-08-26
)

######## Protobuf & Grpc C++ libraries ########

# zlib, required by Protobuf & Grpc. (Patched them to use @com_github_madler_zlib//:zlib)
# required by boost. (Patched com_github_nelhage_rules_boost to use @com_github_madler_zlib//:zlib)
http_archive(
    name = "com_github_madler_zlib",
    build_file = "//third_party/zlib:zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],  # 2017-01-15
)

# abseil, required by Grpc, gtest & kythe.
http_archive(
    name = "com_google_absl",
    patch_args = ["-p1"],
    patches = ["//third_party/abseil-cpp:0001-Rename-Valgrind-related-functions-to-avoid-conflicts.patch"],
    sha256 = "15e1afcb57de3855941927cb0fb1c7cd6194b38a8d2f297e9a8e4e66ff66aa3d",
    strip_prefix = "abseil-cpp-67222ffc4c83d918ce8395aa61769eeb77df4c4d",
    urls = ["https://github.com/abseil/abseil-cpp/archive/67222ffc4c83d918ce8395aa61769eeb77df4c4d.tar.gz"],  # 2019-08-06
)

# boringssl, required by Grpc.
http_archive(
    name = "boringssl",
    sha256 = "78a7f1cf0889de5fc0c85b8c7b940a8fb032f66014d10197ffc1804f20c1d0e4",
    strip_prefix = "boringssl-a21f78d24bf645ccd6774b2c7e52e3c0514f7f29",
    urls = ["https://github.com/google/boringssl/archive/a21f78d24bf645ccd6774b2c7e52e3c0514f7f29.tar.gz"],  # 2019-08-05
)

# gtest, required by Grpc.
http_archive(
    name = "com_google_googletest",
    sha256 = "9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c",
    strip_prefix = "googletest-release-1.8.1",
    urls = ["https://github.com/google/googletest/archive/release-1.8.1.tar.gz"],  # 2018-08-31
)

# gflags, required by Grpc & kythe.
# TODO(zhangshuai.ustc): Patch them to use abseil flags?
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],  # 2018-11-12
)

# Google benchmark, required by Grpc.
http_archive(
    name = "com_github_google_benchmark",
    sha256 = "59f918c8ccd4d74b6ac43484467b500f1d64b40cc1010daa055375b322a43ba3",
    strip_prefix = "benchmark-1.5.0",
    urls = ["https://github.com/google/benchmark/archive/v1.5.0.tar.gz"],  # 2019-05-28
)

# proto_library, cc_proto_library, and java_proto_library rules implicitly
# depend on @com_google_protobuf for protoc and proto runtimes.
http_archive(
    name = "com_google_protobuf",
    patch_args = ["-p1"],
    patches = ["//third_party/protobuf:0001-build-with-com_github_madler_zlib.patch"],
    sha256 = "c90d9e13564c0af85fd2912545ee47b57deded6e5a97de80395b6d2d9be64854",
    strip_prefix = "protobuf-3.9.1",
    urls = ["https://github.com/google/protobuf/archive/v3.9.1.zip"],
)

# java_lite_proto_library rules implicitly depend on @com_google_protobuf_javalite//:javalite_toolchain,
# which is the JavaLite proto runtime (base classes and common utilities).
bind(
    name = "com_google_protobuf_javalite",
    actual = "@com_google_protobuf",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

# grpc
# This needs to be placed after TensorFlow to resolve some bazel conflicts.
http_archive(
    name = "com_github_grpc_grpc",
    patch_args = ["-p1"],
    patches = ["//third_party/grpc:0001-build-with-com_github_madler_zlib.patch"],
    sha256 = "cce1d4585dd017980d4a407d8c5e9f8fc8c1dbb03f249b99e88a387ebb45a035",
    strip_prefix = "grpc-1.22.1",
    urls = ["https://github.com/grpc/grpc/archive/v1.22.1.tar.gz"],  # 2019-08-15
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

######## Grpc Java libraries ########

# grpc-java
http_archive(
    name = "io_grpc_grpc_java",
    patch_args = ["-p1"],
    patches = ["//third_party/grpc-java:0001-fix-build-with-grpc-1.22.0.patch"],
    sha256 = "6e63bd6f5a82de0b84c802390adb8661013bad9ebf910ad7e1f3f72b5f798832",
    strip_prefix = "grpc-java-1.22.1",
    urls = ["https://github.com/grpc/grpc-java/archive/v1.22.1.tar.gz"],  # 2019-08-14
)

# Load the grpc-java dependencies from the local bzl file.
load("@io_grpc_grpc_java//:repositories.bzl", "grpc_java_repositories")

grpc_java_repositories(
    omit_com_google_protobuf = True,
    omit_com_google_protobuf_javalite = True,
)

######## Java toolchains ########

# TODO(zhangshuai.ustc): Load it from a bzl script file for flexibility.
MAVEN_REPOSITORY_URL = "http://repo1.maven.org/maven2/"

# Default Maven Server to a near server
maven_server(
    name = "default",
    url = MAVEN_REPOSITORY_URL,
)

######## Go toolchains ########

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "8df59f11fb697743cbb3f26cfb8750395f30471e9eabde0d174c3aebc7a1cd39",
    urls = ["https://github.com/bazelbuild/rules_go/releases/download/0.19.1/rules_go-0.19.1.tar.gz"],  # 2019-06-20
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

######## Python toolchains ########

# Copied from tensorflow, but removed the numpy dependency.
load("//third_party/py:python_configure.bzl", "python_configure")

python_configure(name = "local_config_python")

######## Ported Source Code Dependencies ########

# The ported source code need go before any other dependencies to prevent overriding.

######## C++ External Dependencies ########

# glog, required by kythe.
http_archive(
    name = "com_github_google_glog",
    sha256 = "f28359aeba12f30d73d9e4711ef356dc842886968112162bc73002645139c39c",
    strip_prefix = "glog-0.4.0",
    urls = ["https://github.com/google/glog/archive/v0.4.0.tar.gz"],
)

# boost repository
http_archive(
    name = "com_github_nelhage_rules_boost",
    patch_args = ["-p1"],
    patches = ["//third_party/boost:0001-build-with-com_github_madler_zlib.patch"],
    sha256 = "23030ce74cd02a10df806c5b26c20af38d2ca06e2b2b2af31517f73d8ce63528",
    strip_prefix = "rules_boost-82ae1790cef07f3fd618592ad227fe2d66fe0b31",
    urls = ["https://github.com/nelhage/rules_boost/archive/82ae1790cef07f3fd618592ad227fe2d66fe0b31.zip"],  # 2019-08-05, Boost 1.68.0
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

# GSL repository
http_archive(
    name = "com_github_microsoft_gsl",
    build_file = "//third_party/gsl:gsl.BUILD",
    sha256 = "b991e8b347b763f4b0e521b9687cdf8aebd6c5a831a6b6435b33fc11007e2c7f",
    strip_prefix = "GSL-1212beae777dba02c230ece8c0c0ec12790047ea",
    urls = ["https://github.com/microsoft/GSL/archive/1212beae777dba02c230ece8c0c0ec12790047ea.zip"],  # 2019-06-13
)

# rapidjson, required by kythe.
http_archive(
    name = "com_github_tencent_rapidjson",
    build_file = "//third_party/rapidjson:rapidjson.BUILD",
    sha256 = "8e00c38829d6785a2dfb951bb87c6974fa07dfe488aa5b25deec4b8bc0f6a3ab",
    strip_prefix = "rapidjson-1.1.0",
    url = "https://github.com/Tencent/rapidjson/archive/v1.1.0.zip",
)

# zookeeper-client-c
http_archive(
    name = "org_apache_zookeeper",
    build_file = "//third_party/zookeeper-client-c:zookeeper-client-c.BUILD",
    sha256 = "b14f7a0fece8bd34c7fffa46039e563ac5367607c612517aa7bd37306afbd1cd",
    strip_prefix = "zookeeper-3.4.14/zookeeper-client/zookeeper-client-c",
    urls = [
        "https://www-eu.apache.org/dist/zookeeper/zookeeper-3.4.14/zookeeper-3.4.14.tar.gz",
        "https://archive.apache.org/dist/zookeeper/zookeeper-3.4.14/zookeeper-3.4.14.tar.gz",  # 2019-04-01
    ],
)

# Required by swig.
http_archive(
    name = "pcre",
    build_file = "//third_party/pcre:pcre.BUILD",
    sha256 = "69acbc2fbdefb955d42a4c606dfde800c2885711d2979e356c0636efde9ec3b5",
    strip_prefix = "pcre-8.42",
    urls = [
        "http://mirror.tensorflow.org/ftp.exim.org/pub/pcre/pcre-8.42.tar.gz",
        "http://ftp.exim.org/pub/pcre/pcre-8.42.tar.gz",  # 2018-03-20
    ],
)

http_archive(
    name = "swig",
    build_file = "@//third_party/swig:swig.BUILD",
    sha256 = "e8a39cd6437e342cdcbd5af27a9bf11b62dc9efec9248065debcb8276fcbb925",
    strip_prefix = "swig-4.0.0",
    urls = [
        "http://ufpr.dl.sourceforge.net/project/swig/swig/swig-4.0.0/swig-4.0.0.tar.gz",
        "http://pilotfiber.dl.sourceforge.net/project/swig/swig/swig-4.0.0/swig-4.0.0.tar.gz",  # 2019-04-29
    ],
)

######## Java External Dependencies ########

RULES_JVM_EXTERNAL_TAG = "2.7"  # 2019-08-15

RULES_JVM_EXTERNAL_SHA = "f04b1466a00a2845106801e0c5cec96841f49ea4e7d1df88dc8e4bf31523df74"

http_archive(
    name = "rules_jvm_external",
    sha256 = RULES_JVM_EXTERNAL_SHA,
    strip_prefix = "rules_jvm_external-%s" % RULES_JVM_EXTERNAL_TAG,
    url = "https://github.com/bazelbuild/rules_jvm_external/archive/%s.zip" % RULES_JVM_EXTERNAL_TAG,
)

load("@rules_jvm_external//:defs.bzl", "maven_install")

# Generate with following commands:
# 1. Copy `dev-support/bin/refresh_maven_dependencies.sh` results here in artifacts.
# 2. bazel run @unpinned_maven//:pin
maven_install(
    artifacts = [
        "ch.qos.logback:logback-classic:1.2.3",
        "commons-cli:commons-cli:1.4",
        "com.google.guava:guava:26.0-jre",
        "javax.annotation:javax.annotation-api:1.3.2",
        "junit:junit:4.12",
        "org.apache.commons:commons-lang3:3.9",
        "org.apache.commons:commons-math3:3.6.1",
        "org.slf4j:slf4j-api:1.7.25",
        "org.assertj:assertj-core:3.13.2",
    ],
    maven_install_json = "@//:maven_install.json",
    repositories = [MAVEN_REPOSITORY_URL],
)

load("@maven//:defs.bzl", "pinned_maven_install")

pinned_maven_install()

######## Go External Dependencies ########

# Generate BUILD files for Go project.
http_archive(
    name = "bazel_gazelle",
    sha256 = "be9296bfd64882e3c08e3283c58fcb461fa6dd3c171764fcc4cf322f60615a9b",
    urls = [
        "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/bazel-gazelle/releases/download/0.18.1/bazel-gazelle-0.18.1.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/0.18.1/bazel-gazelle-0.18.1.tar.gz",  # 2019-04-17
    ],
)

# Generate BUILD files for Go project.
load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()

######## Load Tools After Loading All Others ########

# cpplint
http_archive(
    name = "cpplint",
    build_file_content =
        """
py_binary(
    name = "cpplint",
    srcs = ["cpplint.py"],
    visibility = ["//visibility:public"],
)
""",
    sha256 = "05f879aab5a04307e916e32afb547567d8a44149ddc2f91bf846ce2650ce6d7d",
    strip_prefix = "cpplint-1.4.4",
    urls = [
        "https://github.com/cpplint/cpplint/archive/1.4.4.tar.gz",  # 2019-02-25
    ],
)

# Buildifier tool
http_archive(
    name = "com_github_bazelbuild_buildtools",
    sha256 = "5ec71602e9b458b01717fab1d37492154c1c12ea83f881c745dbd88e9b2098d8",
    strip_prefix = "buildtools-0.28.0",
    url = "https://github.com/bazelbuild/buildtools/archive/0.28.0.tar.gz",  # 2019-07-16
)

load("@com_github_bazelbuild_buildtools//buildifier:deps.bzl", "buildifier_dependencies")

buildifier_dependencies()

# kythe, generate compilation database for C++.
# TODO(zhangshuai.ustc): Build kythe in a lighter way.
http_archive(
    name = "io_kythe",
    sha256 = "4856b9b974ac771e5d8be9f14e7d98f71760ce85c9f20321a94c7ea94d4ab2ee",
    strip_prefix = "kythe-b366059660304a27ae7775055ca168a4ecb55a72",
    urls = ["https://github.com/kythe/kythe/archive/b366059660304a27ae7775055ca168a4ecb55a72.zip"],  # 2019-06-12
)
