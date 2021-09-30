load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

copy_file(
    name = "config_h",
    src = "@//third_party/thrift:config.h",
    out = "config.h",
)

copy_file(
    name = "lib_cpp_config_h",
    src = "@//third_party/thrift:config.h",
    out = "lib/cpp/src/thrift/config.h",
)

copy_file(
    name = "version_h",
    src = "@//third_party/thrift:version.h",
    out = "compiler/cpp/src/thrift/version.h",
)

cc_binary(
    name = "thrift",
    srcs = [
        "compiler/cpp/src/thrift/audit/t_audit.cpp",
        "compiler/cpp/src/thrift/common.cc",
        "compiler/cpp/src/thrift/generate/t_as3_generator.cc",
        "compiler/cpp/src/thrift/generate/t_c_glib_generator.cc",
        "compiler/cpp/src/thrift/generate/t_cl_generator.cc",
        "compiler/cpp/src/thrift/generate/t_cpp_generator.cc",
        "compiler/cpp/src/thrift/generate/t_csharp_generator.cc",
        "compiler/cpp/src/thrift/generate/t_d_generator.cc",
        "compiler/cpp/src/thrift/generate/t_dart_generator.cc",
        "compiler/cpp/src/thrift/generate/t_delphi_generator.cc",
        "compiler/cpp/src/thrift/generate/t_erl_generator.cc",
        "compiler/cpp/src/thrift/generate/t_generator.cc",
        "compiler/cpp/src/thrift/generate/t_go_generator.cc",
        "compiler/cpp/src/thrift/generate/t_gv_generator.cc",
        "compiler/cpp/src/thrift/generate/t_haxe_generator.cc",
        "compiler/cpp/src/thrift/generate/t_hs_generator.cc",
        "compiler/cpp/src/thrift/generate/t_html_generator.cc",
        "compiler/cpp/src/thrift/generate/t_java_generator.cc",
        "compiler/cpp/src/thrift/generate/t_javame_generator.cc",
        "compiler/cpp/src/thrift/generate/t_js_generator.cc",
        "compiler/cpp/src/thrift/generate/t_json_generator.cc",
        "compiler/cpp/src/thrift/generate/t_lua_generator.cc",
        "compiler/cpp/src/thrift/generate/t_netcore_generator.cc",
        "compiler/cpp/src/thrift/generate/t_netstd_generator.cc",
        "compiler/cpp/src/thrift/generate/t_ocaml_generator.cc",
        "compiler/cpp/src/thrift/generate/t_perl_generator.cc",
        "compiler/cpp/src/thrift/generate/t_php_generator.cc",
        "compiler/cpp/src/thrift/generate/t_py_generator.cc",
        "compiler/cpp/src/thrift/generate/t_rb_generator.cc",
        "compiler/cpp/src/thrift/generate/t_rs_generator.cc",
        "compiler/cpp/src/thrift/generate/t_st_generator.cc",
        "compiler/cpp/src/thrift/generate/t_swift_generator.cc",
        "compiler/cpp/src/thrift/generate/t_xml_generator.cc",
        "compiler/cpp/src/thrift/generate/t_xsd_generator.cc",
        "compiler/cpp/src/thrift/main.cc",
        "compiler/cpp/src/thrift/parse/parse.cc",
        "compiler/cpp/src/thrift/parse/t_typedef.cc",
        ":compiler/cpp/src/thrift/version.h",
        ":lib/cpp/src/thrift/config.h",
    ] + glob(
        ["**/*.h"],
    ),
    defines = [
        "HAVE_CONFIG_H",
    ],
    includes = [
        "compiler/cpp/src",
    ],
    linkopts = [
        "-lrt",
        "-pthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":parser",
    ],
)

cc_library(
    name = "libthrift",
    srcs = [
        "lib/cpp/src/thrift/TApplicationException.cpp",
        "lib/cpp/src/thrift/TOutput.cpp",
        "lib/cpp/src/thrift/VirtualProfiling.cpp",
        "lib/cpp/src/thrift/async/TAsyncChannel.cpp",
        "lib/cpp/src/thrift/async/TAsyncProtocolProcessor.cpp",
        "lib/cpp/src/thrift/async/TConcurrentClientSyncInfo.cpp",
        "lib/cpp/src/thrift/concurrency/Monitor.cpp",
        "lib/cpp/src/thrift/concurrency/Mutex.cpp",
        "lib/cpp/src/thrift/concurrency/Thread.cpp",
        "lib/cpp/src/thrift/concurrency/ThreadFactory.cpp",
        "lib/cpp/src/thrift/concurrency/ThreadManager.cpp",
        "lib/cpp/src/thrift/concurrency/TimerManager.cpp",
        "lib/cpp/src/thrift/processor/PeekProcessor.cpp",
        "lib/cpp/src/thrift/protocol/TBase64Utils.cpp",
        "lib/cpp/src/thrift/protocol/TDebugProtocol.cpp",
        "lib/cpp/src/thrift/protocol/TJSONProtocol.cpp",
        "lib/cpp/src/thrift/protocol/TMultiplexedProtocol.cpp",
        "lib/cpp/src/thrift/protocol/TProtocol.cpp",
        "lib/cpp/src/thrift/server/TConnectedClient.cpp",
        "lib/cpp/src/thrift/server/TServer.cpp",
        "lib/cpp/src/thrift/server/TServerFramework.cpp",
        "lib/cpp/src/thrift/server/TSimpleServer.cpp",
        "lib/cpp/src/thrift/server/TThreadPoolServer.cpp",
        "lib/cpp/src/thrift/server/TThreadedServer.cpp",
        "lib/cpp/src/thrift/transport/TBufferTransports.cpp",
        "lib/cpp/src/thrift/transport/TFDTransport.cpp",
        "lib/cpp/src/thrift/transport/TFileTransport.cpp",
        "lib/cpp/src/thrift/transport/THttpClient.cpp",
        "lib/cpp/src/thrift/transport/THttpServer.cpp",
        "lib/cpp/src/thrift/transport/THttpTransport.cpp",
        "lib/cpp/src/thrift/transport/TPipe.cpp",
        "lib/cpp/src/thrift/transport/TPipeServer.cpp",
        "lib/cpp/src/thrift/transport/TSSLServerSocket.cpp",
        "lib/cpp/src/thrift/transport/TSSLSocket.cpp",
        "lib/cpp/src/thrift/transport/TServerSocket.cpp",
        "lib/cpp/src/thrift/transport/TSimpleFileTransport.cpp",
        "lib/cpp/src/thrift/transport/TSocket.cpp",
        "lib/cpp/src/thrift/transport/TSocketPool.cpp",
        "lib/cpp/src/thrift/transport/TTransportException.cpp",
        "lib/cpp/src/thrift/transport/TTransportUtils.cpp",
    ],
    hdrs = glob(
        ["**/*.h"],
        exclude = [
            "lib/cpp/src/thrift/config.h",
        ],
    ) + [
        ":lib/cpp/src/thrift/config.h",
    ],
    defines = [
        "__STDC_FORMAT_MACROS",
        "__STDC_LIMIT_MACROS",
        "HAVE_CONFIG_H",
    ],
    includes = ["lib/cpp/src"],
    textual_hdrs = glob(["**/*.tcc"]),
    visibility = ["//visibility:public"],
    deps = [
        "@boost//:algorithm",
        "@boost//:locale",
        "@boost//:noncopyable",
        "@boost//:numeric_conversion",
        "@boost//:scoped_array",
        "@boost//:smart_ptr",
        "@boost//:tokenizer",
        "@boringssl//:ssl",
    ],
)

cc_library(
    name = "libthriftnb",
    srcs = [
        "lib/cpp/src/thrift/async/TEvhttpClientChannel.cpp",
        "lib/cpp/src/thrift/async/TEvhttpServer.cpp",
        "lib/cpp/src/thrift/server/TNonblockingServer.cpp",
        "lib/cpp/src/thrift/transport/TNonblockingSSLServerSocket.cpp",
        "lib/cpp/src/thrift/transport/TNonblockingServerSocket.cpp",
    ],
    hdrs = glob(
        ["**/*.h"],
        exclude = [
            "lib/cpp/src/thrift/config.h",
        ],
    ) + [
        ":lib/cpp/src/thrift/config.h",
    ],
    defines = [
        "__STDC_FORMAT_MACROS",
        "__STDC_LIMIT_MACROS",
        "HAVE_CONFIG_H",
    ],
    includes = ["lib/cpp/src"],
    textual_hdrs = glob(["**/*.tcc"]),
    visibility = ["//visibility:public"],
    deps = [
        "@boost//:noncopyable",
        "@boost//:numeric_conversion",
        "@boost//:scoped_array",
        "@boost//:smart_ptr",
        "@boringssl//:ssl",
        "@com_github_libevent_libevent//:libevent",
    ],
)

cc_library(
    name = "libthriftz",
    srcs = [
        "lib/cpp/src/thrift/protocol/THeaderProtocol.cpp",
        "lib/cpp/src/thrift/transport/THeaderTransport.cpp",
        "lib/cpp/src/thrift/transport/TZlibTransport.cpp",
    ],
    hdrs = glob(
        ["**/*.h"],
        exclude = [
            "lib/cpp/src/thrift/config.h",
        ],
    ) + [
        ":lib/cpp/src/thrift/config.h",
    ],
    defines = [
        "__STDC_FORMAT_MACROS",
        "__STDC_LIMIT_MACROS",
        "HAVE_CONFIG_H",
    ],
    includes = ["lib/cpp/src"],
    textual_hdrs = glob(["**/*.tcc"]),
    visibility = ["//visibility:public"],
    deps = [
        "@boost//:numeric_conversion",
        "@boost//:scoped_array",
        "@boost//:smart_ptr",
        "@com_github_madler_zlib//:zlib",
    ],
)

cc_library(
    name = "parser",
    srcs = [
        "compiler/cpp/src/thrift/thriftl.cc",
        "compiler/cpp/src/thrift/thrifty.cc",
    ],
    hdrs = glob(
        [
            "**/*.h",
            "**/*.hh",
        ],
        exclude = ["config.h"],
    ),
    copts = [
        "-std=c++11",
        "-Wall",
        "-Wno-sign-compare",
        "-Wno-unused",
    ],
    defines = [
        "HAVE_CONFIG_H",
    ],
    includes = [
        "compiler/cpp/src",
    ],
    deps = [
        "@com_github_westes_flex//:fl",
    ],
)
