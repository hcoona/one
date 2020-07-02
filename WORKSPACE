load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("//bazel:repository_defs.bzl", "bazel_version_repository")

######## Starlark libraries ########

http_archive(
    name = "bazel_skylib",
    sha256 = "e5d90f0ec952883d56747b7604e2a15ee36e288bb556c3d0ed33e818a4d971f2",
    strip_prefix = "bazel-skylib-1.0.2",
    url = "https://github.com/bazelbuild/bazel-skylib/archive/1.0.2.tar.gz",  # 2019-10-10
)

http_archive(
    name = "platforms",
    sha256 = "c0e41126bcb54b6c6d219d019b04ca7c0392e42c186b48c36d4cd5d817bf209e",
    strip_prefix = "platforms-43155b81d40765f0d13008bc77cd2cca8ba9fb2a",
    url = "https://github.com/bazelbuild/platforms/archive/43155b81d40765f0d13008bc77cd2cca8ba9fb2a.tar.gz",  # 2019-08-26
)

# Required by protobuf.
http_archive(
    name = "rules_cc",
    sha256 = "9d48151ea71b3e225adfb6867e6d2c7d0dce46cbdc8710d9a9a628574dfd40a0",
    strip_prefix = "rules_cc-818289e5613731ae410efb54218a4077fb9dbb03",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/818289e5613731ae410efb54218a4077fb9dbb03.tar.gz"],
)

# Required by protobuf.
http_archive(
    name = "rules_java",
    sha256 = "f5a3e477e579231fca27bf202bb0e8fbe4fc6339d63b38ccb87c2760b533d1c3",
    strip_prefix = "rules_java-981f06c3d2bd10225e85209904090eb7b5fb26bd",
    urls = ["https://github.com/bazelbuild/rules_java/archive/981f06c3d2bd10225e85209904090eb7b5fb26bd.tar.gz"],
)

# Required by protobuf.
http_archive(
    name = "rules_proto",
    sha256 = "602e7161d9195e50246177e7c55b2f39950a9cf7366f74ed5f22fd45750cd208",
    strip_prefix = "rules_proto-97d8af4dc474595af3900dd85cb3a29ad28cc313",
    urls = ["https://github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.tar.gz"],
)

# Required by grpc.
http_archive(
    name = "build_bazel_apple_support",
    sha256 = "9114c452eee622598cf9cdc90ecb12b06af7f914f33440b26deba9a9704d450c",
    url = "https://github.com/bazelbuild/apple_support/releases/download/0.7.2/apple_support.0.7.2.tar.gz",  # 2019-10-09
)

# Required by grpc.
http_archive(
    name = "build_bazel_rules_swift",
    sha256 = "ef2578a50a4dae1debb42a41699a8a77d3f31814c097be8d594f7f4d7f9fce14",
    url = "https://github.com/bazelbuild/rules_swift/releases/download/0.13.0/rules_swift.0.13.0.tar.gz",  # 2019-10-09
)

# Required by grpc.
http_archive(
    name = "bazel_toolchains",
    sha256 = "0b36eef8a66f39c8dbae88e522d5bbbef49d5e66e834a982402c79962281be10",
    strip_prefix = "bazel-toolchains-1.0.1",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/1.0.1.tar.gz",
        "https://github.com/bazelbuild/bazel-toolchains/releases/download/1.0.1/bazel-toolchains-1.0.1.tar.gz",
    ],
)

######## Java toolchains ########

RULES_JVM_EXTERNAL_TAG = "bad9e2501279aea5268b1b8a5463ccc1be8ddf65"  # 2020-03-28

RULES_JVM_EXTERNAL_SHA = "8ba00db3da4c65a37050a95ca17551cf0956ef33b0c35f7cc058c5d8f33dd59c"

http_archive(
    name = "rules_jvm_external",
    sha256 = RULES_JVM_EXTERNAL_SHA,
    strip_prefix = "rules_jvm_external-%s" % RULES_JVM_EXTERNAL_TAG,
    url = "https://github.com/bazelbuild/rules_jvm_external/archive/%s.zip" % RULES_JVM_EXTERNAL_TAG,
)

load("@rules_jvm_external//:defs.bzl", "maven_install")

######## Go toolchains ########

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "a8d6b1b354d371a646d2f7927319974e0f9e52f73a2452d2b3877118169eb6bb",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.23.3/rules_go-v0.23.3.tar.gz",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.23.3/rules_go-v0.23.3.tar.gz",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

######## Python toolchains ########

http_archive(
    name = "rules_python",
    sha256 = "b5668cde8bb6e3515057ef465a35ad712214962f0b3a314e551204266c7be90c",
    strip_prefix = "rules_python-0.0.2",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.0.2/rules_python-0.0.2.tar.gz",
)

load("@rules_python//python:pip.bzl", "pip3_import")

######## Protobuf & Grpc C++ libraries ########

# zlib, required by Protobuf & Grpc. (Patched them to use @com_github_madler_zlib//:zlib)
# required by boost. (Patched com_github_nelhage_rules_boost to use @com_github_madler_zlib//:zlib)
# required by rocksdb.
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
    sha256 = "96163b88c5e00b137b12d9ae7d47ef33cf2f14d78ffdd91547e20bda01c1134d",
    strip_prefix = "abseil-cpp-38db52adb2eabc0969195b33b30763e0a1285ef9",
    urls = ["https://github.com/abseil/abseil-cpp/archive/38db52adb2eabc0969195b33b30763e0a1285ef9.tar.gz"],  # 2020-07-02
)

# boringssl, required by Grpc.
http_archive(
    name = "boringssl",
    sha256 = "78a7f1cf0889de5fc0c85b8c7b940a8fb032f66014d10197ffc1804f20c1d0e4",
    strip_prefix = "boringssl-a21f78d24bf645ccd6774b2c7e52e3c0514f7f29",
    urls = ["https://github.com/google/boringssl/archive/a21f78d24bf645ccd6774b2c7e52e3c0514f7f29.tar.gz"],  # 2019-08-05
)

# gtest, required by Grpc, rocksdb.
http_archive(
    name = "com_google_googletest",
    sha256 = "9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c",
    strip_prefix = "googletest-release-1.8.1",
    urls = ["https://github.com/google/googletest/archive/release-1.8.1.tar.gz"],  # 2018-08-31
)

# gflags, required by Grpc, kythe, glog.
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

# required by Grpc
http_archive(
    name = "com_github_cares_cares",
    build_file = "@com_github_grpc_grpc//third_party:cares/cares.BUILD",
    sha256 = "e8c2751ddc70fed9dc6f999acd92e232d5846f009ee1674f8aee81f19b2b915a",
    strip_prefix = "c-ares-e982924acee7f7313b4baa4ee5ec000c5e373c30",
    urls = [
        "https://storage.googleapis.com/grpc-bazel-mirror/github.com/c-ares/c-ares/archive/e982924acee7f7313b4baa4ee5ec000c5e373c30.tar.gz",
        "https://github.com/c-ares/c-ares/archive/e982924acee7f7313b4baa4ee5ec000c5e373c30.tar.gz",
    ],
)

# required by Grpc
http_archive(
    name = "io_opencensus_cpp",
    sha256 = "90d6fafa8b1a2ea613bf662731d3086e1c2ed286f458a95c81744df2dbae41b1",
    strip_prefix = "opencensus-cpp-c9a4da319bc669a772928ffc55af4a61be1a1176",
    urls = [
        "https://storage.googleapis.com/grpc-bazel-mirror/github.com/census-instrumentation/opencensus-cpp/archive/c9a4da319bc669a772928ffc55af4a61be1a1176.tar.gz",
        "https://github.com/census-instrumentation/opencensus-cpp/archive/c9a4da319bc669a772928ffc55af4a61be1a1176.tar.gz",
    ],
)

# required by Grpc
http_archive(
    name = "upb",
    sha256 = "e9c136e56b98c8eb48ad1c9f8df4a6348e99f9f336ee6199c4259a312c2e3598",
    strip_prefix = "upb-d8f3d6f9d415b31f3ce56d46791706c38fa311bc",
    url = "https://github.com/protocolbuffers/upb/archive/d8f3d6f9d415b31f3ce56d46791706c38fa311bc.tar.gz",
)

# required by Grpc
http_archive(
    name = "libuv",
    build_file = "@com_github_grpc_grpc//third_party:libuv.BUILD",
    sha256 = "dfb4fe1ff0b47340978490a14bf253475159ecfcbad46ab2a350c78f9ce3360f",
    strip_prefix = "libuv-15ae750151ac9341e5945eb38f8982d59fb99201",
    urls = [
        "https://storage.googleapis.com/grpc-bazel-mirror/github.com/libuv/libuv/archive/15ae750151ac9341e5945eb38f8982d59fb99201.tar.gz",
        "https://github.com/libuv/libuv/archive/15ae750151ac9341e5945eb38f8982d59fb99201.tar.gz",
    ],
)

# Required by upb, which is required by protobuf.
bazel_version_repository(
    name = "upb_bazel_version",
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

# grpc
# This needs to be placed after TensorFlow to resolve some bazel conflicts.
http_archive(
    name = "com_github_grpc_grpc",
    patch_args = ["-p1"],
    patches = ["//third_party/grpc:v1.29.1.patch"],
    sha256 = "0343e6dbde66e9a31c691f2f61e98d79f3584e03a11511fad3f10e3667832a45",
    strip_prefix = "grpc-1.29.1",
    urls = ["https://github.com/grpc/grpc/archive/v1.29.1.tar.gz"],  # 2020-05-21
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

######## Grpc Java libraries ########

# grpc-java
http_archive(
    name = "io_grpc_grpc_java",
    sha256 = "15f655349f174c9258b6dbb27a9bd1cdae211ee2bf8b27b7d56ef232c191ee26",
    strip_prefix = "grpc-java-1.30.0",
    urls = ["https://github.com/grpc/grpc-java/archive/v1.30.0.tar.gz"],  # 2020-06-09
)

# Load the grpc-java dependencies from the local bzl file.
load("@io_grpc_grpc_java//:repositories.bzl", "IO_GRPC_GRPC_JAVA_ARTIFACTS", "IO_GRPC_GRPC_JAVA_OVERRIDE_TARGETS")

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

# bzip2, required by rocksdb.
http_archive(
    name = "org_sourceware_bzip2",
    build_file = "//third_party/bzip2:bzip2.BUILD",
    sha256 = "ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269",
    strip_prefix = "bzip2-1.0.8",
    urls = ["https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"],  # 2019-06-13
)

# lz4, required by rocksdb.
http_archive(
    name = "com_github_lz4_lz4",
    build_file = "//third_party/lz4:lz4.BUILD",
    sha256 = "658ba6191fa44c92280d4aa2c271b0f4fbc0e34d249578dd05e50e76d0e5efcc",
    strip_prefix = "lz4-1.9.2",
    urls = ["https://github.com/lz4/lz4/archive/v1.9.2.tar.gz"],  # 2019-08-20
)

# snappy, required by rocksdb.
http_archive(
    name = "com_github_google_snappy",
    build_file = "//third_party/snappy:snappy.BUILD",
    sha256 = "3dfa02e873ff51a11ee02b9ca391807f0c8ea0529a4924afa645fbf97163f9d4",
    strip_prefix = "snappy-1.1.7",
    urls = [
        "https://storage.googleapis.com/mirror.tensorflow.org/github.com/google/snappy/archive/1.1.7.tar.gz",
        "https://github.com/google/snappy/archive/1.1.7.tar.gz",  # 2017-08-25
    ],
)

# zstd, required by rocksdb.
http_archive(
    name = "com_github_facebook_zstd",
    build_file = "//third_party/zstd:zstd.BUILD",
    sha256 = "5eda3502ecc285c3c92ee0cc8cd002234dee39d539b3f692997a0e80de1d33de",
    strip_prefix = "zstd-1.4.3/lib",
    urls = ["https://github.com/facebook/zstd/archive/v1.4.3.tar.gz"],  # 2019-08-20
)

# rocksdb
http_archive(
    name = "com_github_facebook_rocksdb",
    build_file = "//third_party/rocksdb:rocksdb.BUILD",
    sha256 = "3e7365cb2a35982e95e5e5dd0b3352dc78573193dafca02788572318c38483fb",
    strip_prefix = "rocksdb-6.2.2",
    urls = ["https://github.com/facebook/rocksdb/archive/v6.2.2.tar.gz"],  # 2019-08-12
)

# required by brpc
http_archive(
    name = "com_github_google_leveldb",
    build_file = "//third_party/leveldb:leveldb.BUILD",
    sha256 = "3912ac36dbb264a62797d68687711c8024919640d89b6733f9342ada1d16cda1",
    strip_prefix = "leveldb-a53934a3ae1244679f812d998a4f16f2c7f309a6",
    url = "https://github.com/google/leveldb/archive/a53934a3ae1244679f812d998a4f16f2c7f309a6.tar.gz",
)

http_archive(
    name = "com_github_brpc_brpc",
    patch_args = ["-p1"],
    patches = [
        "//third_party/brpc:0001-remove-system-libs.patch",
    ],
    sha256 = "722cd342baf3b05189ca78ecf6c56ea6ffec22e62fc2938335e4e5bab545a49c",
    strip_prefix = "incubator-brpc-0.9.7",
    url = "https://github.com/apache/incubator-brpc/archive/0.9.7.tar.gz",
)

# required by thrift
http_archive(
    name = "com_github_westes_flex",
    build_file = "//third_party/flex:flex.BUILD",
    patch_args = ["-p1"],
    patches = [
        "//third_party/flex:0001-fix-include-config.patch",
    ],
    sha256 = "e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995",
    strip_prefix = "flex-2.6.4",
    url = "https://github.com/westes/flex/files/981163/flex-2.6.4.tar.gz",
)

http_archive(
    name = "org_apache_thrift",
    build_file = "//third_party/thrift:thrift.BUILD",
    patch_args = ["-p1"],
    patches = [
        "//third_party/thrift:0001-build-with-boringssl.patch",
        "//third_party/thrift:0002-fix-include.patch",
    ],
    sha256 = "7ad348b88033af46ce49148097afe354d513c1fca7c607b59c33ebb6064b5179",
    strip_prefix = "thrift-0.13.0",
    url = "http://archive.apache.org/dist/thrift/0.13.0/thrift-0.13.0.tar.gz",
)

# msgpack-c
http_archive(
    name = "com_github_msgpack_msgpack_c",
    build_file = "//third_party/msgpack-c:msgpack.BUILD",
    patch_args = ["-p1"],
    patches = [
        "//third_party/msgpack-c:0001-add-version-h.patch",
        "//third_party/msgpack-c:0002-fix-test-cmath.patch",
        "//third_party/msgpack-c:0003-bugfix-raw-ne.patch",
        "//third_party/msgpack-c:0004-use-unique-ptr-instead-of-auto-ptr.patch",
    ],
    sha256 = "f86bf09323770825cc1c8cc49790cd4eafef6e6f1c9e70cfb3f9159c3501c277",
    strip_prefix = "msgpack-c-cpp-0.5.9",
    urls = [
        "https://github.com/msgpack/msgpack-c/archive/cpp-0.5.9.tar.gz",
    ],
)  # libcurl, required by consul

http_archive(
    name = "com_github_curl_curl",
    build_file = "//third_party/curl:curl.BUILD",
    sha256 = "01ae0c123dee45b01bbaef94c0bc00ed2aec89cb2ee0fd598e0d302a6b5e0a98",
    strip_prefix = "curl-7.69.1",
    urls = [
        "https://curl.haxx.se/download/curl-7.69.1.tar.gz",
    ],
)

# apr, required by log4cxx
http_archive(
    name = "org_apache_apr",
    build_file = "//third_party/apr:apr.BUILD",
    sha256 = "48e9dbf45ae3fdc7b491259ffb6ccf7d63049ffacbc1c0977cced095e4c2d5a2",
    strip_prefix = "apr-1.7.0",
    urls = [
        "https://downloads.apache.org/apr/apr-1.7.0.tar.gz",
    ],
)

# apr-util, required by log4cxx
http_archive(
    name = "org_apache_apr_util",
    build_file = "//third_party/apr_util:apr_util.BUILD",
    sha256 = "b65e40713da57d004123b6319828be7f1273fbc6490e145874ee1177e112c459",
    strip_prefix = "apr-util-1.6.1",
    urls = [
        "https://downloads.apache.org/apr/apr-util-1.6.1.tar.gz",
    ],
)

# log4cxx, required by bmq
http_archive(
    name = "org_apache_logging_log4cxx",
    build_file = "//third_party/log4cxx:log4cxx.BUILD",
    patch_args = ["-p1"],
    patches = [
        "//third_party/log4cxx:0001-fix-cstring.patch",
        "//third_party/log4cxx:0002-fix-narrowing.patch",
    ],
    sha256 = "0de0396220a9566a580166e66b39674cb40efd2176f52ad2c65486c99c920c8c",
    strip_prefix = "apache-log4cxx-0.10.0",
    urls = [
        "https://downloads.apache.org/logging/log4cxx/0.10.0/apache-log4cxx-0.10.0.tar.gz",
    ],
)

# hiredis
http_archive(
    name = "com_redis_hiredis",
    build_file = "//third_party/hiredis:hiredis.BUILD",
    sha256 = "ff7b2849e55bf3589eecced7125934feb9645c36a4d490d001dc08c93553eafd",
    strip_prefix = "hiredis-0.11.0",
    urls = [
        "https://github.com/redis/hiredis/archive/v0.11.0.tar.gz",
    ],
)

# sasl2
http_archive(
    name = "org_cyrusimap_sasl_sasl",
    build_file = "//third_party/cyrus_sasl:sasl2.BUILD",
    patch_args = ["-p1"],
    patches = [
        "//third_party/cyrus_sasl:0001-fix-include-config.patch",
    ],
    sha256 = "26866b1549b00ffd020f188a43c258017fa1c382b3ddadd8201536f72efb05d5",
    strip_prefix = "cyrus-sasl-2.1.27",
    urls = [
        "https://github.com/cyrusimap/cyrus-sasl/releases/download/cyrus-sasl-2.1.27/cyrus-sasl-2.1.27.tar.gz",
    ],
)

# libevent
http_archive(
    name = "com_github_libevent_libevent",
    build_file = "//third_party/libevent:libevent.BUILD",
    sha256 = "a65bac6202ea8c5609fd5c7e480e6d25de467ea1917c08290c521752f147283d",
    strip_prefix = "libevent-2.1.11-stable",
    urls = [
        "https://github.com/libevent/libevent/releases/download/release-2.1.11-stable/libevent-2.1.11-stable.tar.gz",
    ],
)

http_archive(
    name = "com_googlesource_code_re2",
    sha256 = "428735aea6e8007c14d2e75c69f66cf5b71a009d9933ba6e51e3cbeb52f12f7d",
    strip_prefix = "re2-b83705e2d297f21b0805202bac86c935670634f8",
    url = "https://github.com/google/re2/archive/b83705e2d297f21b0805202bac86c935670634f8.tar.gz",  # 2020-07-01
)

######## Java External Dependencies ########

MAVEN_REPOSITORY_URL = "https://maven.aliyun.com/repository/public"

load("@//bazel:junit5.bzl", "JUNIT5_ARTIFACTS")

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
        "net.java.dev.jna:jna:5.4.0",
        "net.java.dev.jna:jna-platform:5.4.0",
        "org.apache.commons:commons-lang3:3.9",
        "org.apache.commons:commons-math3:3.6.1",
        "org.slf4j:slf4j-api:1.7.25",
        "org.assertj:assertj-core:3.13.2",
        # TODO(zhangshuai.ustc): Add it to Maven.
        "commons-codec:commons-codec:1.13",  # directory_manifest
        "com.google.googlejavaformat:google-java-format:1.7",  # tools/google-java-format
    ] + JUNIT5_ARTIFACTS + IO_GRPC_GRPC_JAVA_ARTIFACTS,
    maven_install_json = "//:maven_install.json",
    override_targets = IO_GRPC_GRPC_JAVA_OVERRIDE_TARGETS,
    repositories = [MAVEN_REPOSITORY_URL],
)

load("@maven//:defs.bzl", "pinned_maven_install")

pinned_maven_install()

######## Go External Dependencies ########

# Generate BUILD files for Go project.
http_archive(
    name = "bazel_gazelle",
    sha256 = "cdb02a887a7187ea4d5a27452311a75ed8637379a1287d8eeb952138ea485f7d",
    urls = [
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.21.1/bazel-gazelle-v0.21.1.tar.gz",  # 2020-05-29
    ],
)

# Generate BUILD files for Go project.
load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()

######## Python External Dependencies ########

pip3_import(
    name = "one_pip_deps",
    requirements = "//:requirements.txt",
)

load("@one_pip_deps//:requirements.bzl", "pip_install")

pip_install()

######## Load Tools After Loading All Others ########

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

# swig
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

# xxd
http_file(
    name = "xxd",
    sha256 = "2d4c4f98726467fc023de6d3ca8da3795ec5d8fbf9d92bb8c29d744877d4cbf8",
    urls = ["https://raw.githubusercontent.com/vim/vim/v8.1.1933/src/xxd/xxd.c"],  # 2019-08-28
)

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
    strip_prefix = "buildtools-3.2.1",
    url = "https://github.com/bazelbuild/buildtools/archive/3.2.1.tar.gz",  # 2020-06-17
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
