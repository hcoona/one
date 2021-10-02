load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

ARROW_FLAGS = [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/cmake_modules/SetupCxxFlags.cmake#L264
    "-Wall",
    "-Wno-conversion",
    "-Wno-deprecated-declarations",
    "-Wno-sign-conversion",
    "-Wno-unused-variable",
    # Add by me to suppress warnings
    "-Wno-documentation-unknown-command",
    "-Wno-shadow-field",
    "-Wno-sign-compare",
    "-Wno-unused-parameter",
] + select({
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/cmake_modules/SetupCxxFlags.cmake#L429
    "@//config:enable_sse42": ["-DARROW_HAVE_SSE4_2"],
    "//conditions:default": [],
}) + select({
    "@//config:enable_avx2": ["-DARROW_HAVE_AVX2"],
    "//conditions:default": [],
}) + select({
    "@//config:enable_bmi2": ["-DARROW_HAVE_BMI2"],
    "//conditions:default": [],
}) + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L265
    "-DURI_STATIC_BUILD",
] + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L272
    "-DARROW_WITH_BROTLI",
    "-DARROW_WITH_BZ2",
    "-DARROW_WITH_LZ4",
    "-DARROW_WITH_SNAPPY",
    "-DARROW_WITH_ZLIB",
    "-DARROW_WITH_ZSTD",
] + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L418
    "-DARROW_HDFS",
] + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L524
    "-DARROW_WITH_BACKTRACE",
] + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/CMakeLists.txt#L730
    "-DARROW_USE_GLOG",
]

#
# Arrow
#

# Extract SRCS variables from
# https://github.com/apache/arrow/blob/apache-arrow-4.0.0/cpp/src/arrow/CMakeLists.txt

ARROW_SRCS = [
    "array/array_base.cc",
    "array/array_binary.cc",
    "array/array_decimal.cc",
    "array/array_dict.cc",
    "array/array_nested.cc",
    "array/array_primitive.cc",
    "array/builder_adaptive.cc",
    "array/builder_base.cc",
    "array/builder_binary.cc",
    "array/builder_decimal.cc",
    "array/builder_dict.cc",
    "array/builder_nested.cc",
    "array/builder_primitive.cc",
    "array/builder_union.cc",
    "array/concatenate.cc",
    "array/data.cc",
    "array/diff.cc",
    "array/util.cc",
    "array/validate.cc",
    "builder.cc",
    "buffer.cc",
    "chunked_array.cc",
    "compare.cc",
    "config.cc",
    "datum.cc",
    "device.cc",
    "extension_type.cc",
    "memory_pool.cc",
    "pretty_print.cc",
    "record_batch.cc",
    "result.cc",
    "scalar.cc",
    "sparse_tensor.cc",
    "status.cc",
    "table.cc",
    "table_builder.cc",
    "tensor.cc",
    "tensor/coo_converter.cc",
    "tensor/csf_converter.cc",
    "tensor/csx_converter.cc",
    "type.cc",
    "visitor.cc",
    "c/bridge.cc",
    "io/buffered.cc",
    "io/caching.cc",
    "io/compressed.cc",
    "io/file.cc",
    "io/hdfs.cc",
    "io/hdfs_internal.cc",
    "io/interfaces.cc",
    "io/memory.cc",
    "io/slow.cc",
    "io/transform.cc",
    "util/basic_decimal.cc",
    "util/bit_block_counter.cc",
    "util/bit_run_reader.cc",
    "util/bit_util.cc",
    "util/bitmap.cc",
    "util/bitmap_builders.cc",
    "util/bitmap_ops.cc",
    "util/bpacking.cc",
    "util/cancel.cc",
    "util/compression.cc",
    "util/cpu_info.cc",
    "util/decimal.cc",
    "util/delimiting.cc",
    "util/formatting.cc",
    "util/future.cc",
    "util/int_util.cc",
    "util/io_util.cc",
    "util/logging.cc",
    "util/key_value_metadata.cc",
    "util/memory.cc",
    "util/mutex.cc",
    "util/string.cc",
    "util/string_builder.cc",
    "util/task_group.cc",
    "util/tdigest.cc",
    "util/thread_pool.cc",
    "util/time.cc",
    "util/trie.cc",
    "util/uri.cc",
    "util/utf8.cc",
    "util/value_parsing.cc",
    "vendored/base64.cpp",
    "vendored/datetime/tz.cpp",
    "vendored/double-conversion/bignum.cc",
    "vendored/double-conversion/double-conversion.cc",
    "vendored/double-conversion/bignum-dtoa.cc",
    "vendored/double-conversion/fast-dtoa.cc",
    "vendored/double-conversion/cached-powers.cc",
    "vendored/double-conversion/fixed-dtoa.cc",
    "vendored/double-conversion/diy-fp.cc",
    "vendored/double-conversion/strtod.cc",
] + select({
    "@//config:enable_avx2": ["util/bpacking_avx2.cc"],
    "//conditions:default": [],
}) + [
    # Need compression libraries
    "util/compression_brotli.cc",
    "util/compression_bz2.cc",
    "util/compression_lz4.cc",
    "util/compression_snappy.cc",
    "util/compression_zlib.cc",
    "util/compression_zstd.cc",
]

ARROW_C_SRCS = [
    "vendored/musl/strptime.c",
    "vendored/uriparser/UriCommon.c",
    "vendored/uriparser/UriCompare.c",
    "vendored/uriparser/UriEscape.c",
    "vendored/uriparser/UriFile.c",
    "vendored/uriparser/UriIp4Base.c",
    "vendored/uriparser/UriIp4.c",
    "vendored/uriparser/UriMemory.c",
    "vendored/uriparser/UriNormalizeBase.c",
    "vendored/uriparser/UriNormalize.c",
    "vendored/uriparser/UriParseBase.c",
    "vendored/uriparser/UriParse.c",
    "vendored/uriparser/UriQuery.c",
    "vendored/uriparser/UriRecompose.c",
    "vendored/uriparser/UriResolve.c",
    "vendored/uriparser/UriShorten.c",
]

ARROW_CSV_SRCS = [
    "csv/converter.cc",
    "csv/chunker.cc",
    "csv/column_builder.cc",
    "csv/column_decoder.cc",
    "csv/options.cc",
    "csv/parser.cc",
    "csv/reader.cc",
]

ARROW_COMPUTE_SRCS = [
    "compute/api_aggregate.cc",
    "compute/api_scalar.cc",
    "compute/api_vector.cc",
    "compute/cast.cc",
    "compute/exec.cc",
    "compute/function.cc",
    "compute/kernel.cc",
    "compute/registry.cc",
    "compute/kernels/aggregate_basic.cc",
    "compute/kernels/aggregate_mode.cc",
    "compute/kernels/aggregate_quantile.cc",
    "compute/kernels/aggregate_tdigest.cc",
    "compute/kernels/aggregate_var_std.cc",
    "compute/kernels/codegen_internal.cc",
    "compute/kernels/hash_aggregate.cc",
    "compute/kernels/scalar_arithmetic.cc",
    "compute/kernels/scalar_boolean.cc",
    "compute/kernels/scalar_cast_boolean.cc",
    "compute/kernels/scalar_cast_internal.cc",
    "compute/kernels/scalar_cast_nested.cc",
    "compute/kernels/scalar_cast_numeric.cc",
    "compute/kernels/scalar_cast_string.cc",
    "compute/kernels/scalar_cast_temporal.cc",
    "compute/kernels/scalar_compare.cc",
    "compute/kernels/scalar_nested.cc",
    "compute/kernels/scalar_set_lookup.cc",
    "compute/kernels/scalar_string.cc",
    "compute/kernels/scalar_validity.cc",
    "compute/kernels/scalar_fill_null.cc",
    "compute/kernels/util_internal.cc",
    "compute/kernels/vector_hash.cc",
    "compute/kernels/vector_nested.cc",
    "compute/kernels/vector_selection.cc",
    "compute/kernels/vector_sort.cc",
] + select({
    "@//config:enable_avx2": ["compute/kernels/aggregate_basic_avx2.cc"],
    "//conditions:default": [],
})

ARROW_DATASET_SRCS = [
    # Extract SRCS variables from
    # https://github.com/apache/arrow/blob/apache-arrow-4.0.0/cpp/src/arrow/dataset/CMakeLists.txt
    "dataset/dataset.cc",
    "dataset/discovery.cc",
    "dataset/expression.cc",
    "dataset/file_base.cc",
    "dataset/file_ipc.cc",
    "dataset/partition.cc",
    "dataset/projector.cc",
    "dataset/scanner.cc",
] + [
    "dataset/file_csv.cc",
    # Move parquet support into parquet library.
]

ARROW_FILESYSTEM_SRCS = [
    "filesystem/filesystem.cc",
    "filesystem/localfs.cc",
    "filesystem/mockfs.cc",
    "filesystem/path_util.cc",
    "filesystem/util_internal.cc",
    "filesystem/hdfs.cc",
]

ARROW_IPC_SRCS = [
    "ipc/dictionary.cc",
    "ipc/feather.cc",
    "ipc/message.cc",
    "ipc/metadata_internal.cc",
    "ipc/options.cc",
    "ipc/reader.cc",
    "ipc/writer.cc",
] + [
    # Need to include ARROW_JSON_SRCS as well.
    "ipc/json_simple.cc",
]

ARROW_JSON_SRCS = [
    "json/options.cc",
    "json/chunked_builder.cc",
    "json/chunker.cc",
    "json/converter.cc",
    "json/object_parser.cc",
    "json/object_writer.cc",
    "json/parser.cc",
    "json/reader.cc",
]

copy_file(
    name = "config_h",
    src = "@com_github_hcoona_one//third_party/arrow:config.h",
    out = "cpp/src/arrow/util/config.h",
    allow_symlink = True,
)

cc_library(
    name = "arrow",
    srcs = [
        "cpp/src/arrow/" + f
        for f in ARROW_SRCS + ARROW_C_SRCS + ARROW_CSV_SRCS + ARROW_COMPUTE_SRCS +
                 ARROW_DATASET_SRCS + ARROW_FILESYSTEM_SRCS + ARROW_IPC_SRCS + ARROW_JSON_SRCS
    ] + glob([
        "cpp/thirdparty/**/*.h",
    ]),
    hdrs = glob([
        "cpp/src/arrow/*.h",
        "cpp/src/arrow/**/*.h",
        "cpp/src/arrow/vendored/*.hpp",
    ]) + [
        "cpp/src/generated/feather_generated.h",
        "cpp/src/generated/File_generated.h",
        "cpp/src/generated/Message_generated.h",
        "cpp/src/generated/Schema_generated.h",
        "cpp/src/generated/SparseTensor_generated.h",
        "cpp/src/generated/Tensor_generated.h",
    ] + [
        ":config_h",
    ],
    copts = ARROW_FLAGS,
    defines = [
        # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L515
        "ARROW_STATIC",
    ],
    includes = [
        "cpp/src",
        "cpp/thirdparty/hadoop/include",
    ],
    linkopts = [
        "-ldl",
    ],
    strip_include_prefix = "cpp/src",
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_facebook_zstd//:zstd",
        "@com_github_google_flatbuffers//:flatbuffers",
        "@com_github_google_glog//:glog",
        "@com_github_google_snappy//:snappy",
        "@com_github_lz4_lz4//:lz4",
        "@com_github_madler_zlib//:zlib",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_github_xtensor_stack_xsimd//:xsimd",
        "@org_brotli//:brotlidec",
        "@org_brotli//:brotlienc",
        "@org_sourceware_bzip2//:bz2",
    ],
)

ARROW_TESTING_SRCS = [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L297
    "io/test_common.cc",
    "ipc/test_common.cc",
    "testing/json_integration.cc",
    "testing/json_internal.cc",
    "testing/gtest_util.cc",
    "testing/random.cc",
    "testing/generator.cc",
    "testing/util.cc",
] + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L360
    "csv/test_common.cc",
] + [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/CMakeLists.txt#L441
    "filesystem/test_util.cc",
]

cc_library(
    name = "arrow_test_common",
    testonly = 1,
    srcs = ["cpp/src/arrow/" + f for f in ARROW_TESTING_SRCS],
    copts = ARROW_FLAGS,
    deps = [
        ":arrow",
        "@com_google_googletest//:gtest",
    ],
)

# Generated from add_arrow_test.

cc_test(
    name = "array_test",
    srcs = [
        "cpp/src/arrow/array/array_binary_test.cc",
        "cpp/src/arrow/array/array_dict_test.cc",
        "cpp/src/arrow/array/array_list_test.cc",
        "cpp/src/arrow/array/array_struct_test.cc",
        "cpp/src/arrow/array/array_test.cc",
        "cpp/src/arrow/array/array_union_test.cc",
        "cpp/src/arrow/array/array_view_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "buffer_test",
    srcs = [
        "cpp/src/arrow/buffer_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "extension_type_test",
    srcs = [
        "cpp/src/arrow/extension_type_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "misc_test",
    srcs = [
        "cpp/src/arrow/datum_test.cc",
        "cpp/src/arrow/memory_pool_test.cc",
        "cpp/src/arrow/pretty_print_test.cc",
        "cpp/src/arrow/result_test.cc",
        "cpp/src/arrow/status_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "public_api_test",
    srcs = [
        "cpp/src/arrow/public_api_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "scalar_test",
    srcs = [
        "cpp/src/arrow/scalar_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "type_test",
    srcs = [
        "cpp/src/arrow/type_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "table_test",
    srcs = [
        "cpp/src/arrow/chunked_array_test.cc",
        "cpp/src/arrow/record_batch_test.cc",
        "cpp/src/arrow/table_builder_test.cc",
        "cpp/src/arrow/table_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "tensor_test",
    srcs = [
        "cpp/src/arrow/tensor_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "sparse_tensor_test",
    srcs = [
        "cpp/src/arrow/sparse_tensor_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stl_iterator_test",
    srcs = [
        "cpp/src/arrow/stl_iterator_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stl_test",
    srcs = [
        "cpp/src/arrow/stl_test.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "builder_benchmark",
    size = "large",
    srcs = [
        "cpp/src/arrow/builder_benchmark.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)

cc_test(
    name = "compare_benchmark",
    size = "large",
    srcs = [
        "cpp/src/arrow/compare_benchmark.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)

cc_test(
    name = "memory_pool_benchmark",
    size = "large",
    srcs = [
        "cpp/src/arrow/memory_pool_benchmark.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)

cc_test(
    name = "type_benchmark",
    size = "large",
    srcs = [
        "cpp/src/arrow/type_benchmark.cc",
    ],
    deps = [
        ":arrow",
        ":arrow_test_common",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)

#
# Parquet
#

PARQUET_SRCS = [
    # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/parquet/CMakeLists.txt#L148
    "arrow/path_internal.cc",
    "arrow/reader.cc",
    "arrow/reader_internal.cc",
    "arrow/schema.cc",
    "arrow/schema_internal.cc",
    "arrow/writer.cc",
    "bloom_filter.cc",
    "column_reader.cc",
    "column_scanner.cc",
    "column_writer.cc",
    "encoding.cc",
    "encryption/encryption.cc",
    "encryption/internal_file_decryptor.cc",
    "encryption/internal_file_encryptor.cc",
    "exception.cc",
    "file_reader.cc",
    "file_writer.cc",
    "level_comparison.cc",
    "level_conversion.cc",
    "metadata.cc",
    "murmur3.cc",
    "platform.cc",
    "printer.cc",
    "properties.cc",
    "schema.cc",
    "statistics.cc",
    "stream_reader.cc",
    "stream_writer.cc",
    "types.cc",
] + select({
    # https://github.com/apache/arrow/blob/d613aa68789288d3503dfbd8376a41f2d28b6c9d/cpp/src/parquet/CMakeLists.txt#L207
    "@//config:enable_avx2": ["level_comparison_avx2.cc"],
    "//conditions:default": [],
}) + select({
    # https://github.com/apache/arrow/blob/d613aa68789288d3503dfbd8376a41f2d28b6c9d/cpp/src/parquet/CMakeLists.txt#L207
    "@//config:enable_bmi2": ["level_conversion_bmi2.cc"],
    "//conditions:default": [],
}) + [
    # https://github.com/apache/arrow/blob/d613aa68789288d3503dfbd8376a41f2d28b6c9d/cpp/src/parquet/CMakeLists.txt#L228
    "encryption/encryption_internal.cc",
]

copy_file(
    name = "parquet_version_h",
    src = "@com_github_hcoona_one//third_party/arrow:parquet_version.h",
    out = "cpp/src/parquet/parquet_version.h",
    allow_symlink = True,
)

cc_library(
    name = "parquet",
    srcs = ["cpp/src/parquet/" + f for f in PARQUET_SRCS] + [
        "cpp/src/generated/parquet_types.cpp",
        "cpp/src/generated/parquet_types.h",
        "cpp/src/generated/parquet_constants.cpp",
        "cpp/src/generated/parquet_constants.h",
    ] + [
        # https://github.com/apache/arrow/blob/f959141ece4d660bce5f7fa545befc0116a7db79/cpp/src/arrow/dataset/CMakeLists.txt#L120
        "cpp/src/arrow/dataset/file_parquet.cc",
    ],
    hdrs = glob([
        "cpp/src/parquet/*.h",
        "cpp/src/parquet/**/*.h",
    ]) + [
        ":parquet_version_h",
    ],
    copts = ARROW_FLAGS,
    includes = ["cpp/src"],
    strip_include_prefix = "cpp/src",
    visibility = ["//visibility:public"],
    deps = [
        ":arrow",
        "@org_apache_thrift//:libthrift",
    ],
)

# Generated from add_parquet_test

# Lack of parquet-testing data folder.
# cc_test(
#     name = "internals-test",
#     srcs = [
#         "cpp/src/parquet/bloom_filter_test.cc",
#         "cpp/src/parquet/deprecated_io_test.cc",
#         "cpp/src/parquet/encoding_test.cc",
#         "cpp/src/parquet/metadata_test.cc",
#         "cpp/src/parquet/properties_test.cc",
#         "cpp/src/parquet/public_api_test.cc",
#         "cpp/src/parquet/statistics_test.cc",
#         "cpp/src/parquet/test_util.cc",
#         "cpp/src/parquet/types_test.cc",
#     ],
#     deps = [
#         ":arrow_test_common",
#         ":parquet",
#         "@com_google_googletest//:gtest_main",
#     ],
# )

# cc_test(
#     name = "reader_test",
#     srcs = [
#         "cpp/src/parquet/column_reader_test.cc",
#         "cpp/src/parquet/column_scanner_test.cc",
#         "cpp/src/parquet/level_conversion_test.cc",
#         "cpp/src/parquet/reader_test.cc",
#         "cpp/src/parquet/stream_reader_test.cc",
#         "cpp/src/parquet/test_util.cc",
#     ],
#     deps = [
#         ":arrow_test_common",
#         ":parquet",
#         "@com_google_googletest//:gtest_main",
#     ],
# )

cc_test(
    name = "writer_test",
    srcs = [
        "cpp/src/parquet/column_writer_test.cc",
        "cpp/src/parquet/file_serialize_test.cc",
        "cpp/src/parquet/stream_writer_test.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_google_googletest//:gtest_main",
    ],
)

# Lack of parquet-testing data folder.
# cc_test(
#     name = "arrow-test",
#     srcs = [
#         "cpp/src/parquet/arrow/arrow_reader_writer_test.cc",
#         "cpp/src/parquet/arrow/arrow_schema_test.cc",
#         "cpp/src/parquet/test_util.cc",
#     ],
#     deps = [
#         ":arrow_test_common",
#         ":parquet",
#         "@com_google_googletest//:gtest_main",
#     ],
# )

cc_test(
    name = "arrow-internals-test",
    srcs = [
        "cpp/src/parquet/arrow/path_internal_test.cc",
        "cpp/src/parquet/arrow/reconstruct_internal_test.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_google_googletest//:gtest_main",
    ],
)

# Lack of parquet-testing data folder.
# cc_test(
#     name = "encryption-test",
#     srcs = [
#         "cpp/src/parquet/encryption_properties_test.cc",
#         "cpp/src/parquet/encryption_read_configurations_test.cc",
#         "cpp/src/parquet/encryption_write_configurations_test.cc",
#         "cpp/src/parquet/test_util.cc",
#     ],
#     deps = [
#         ":arrow_test_common",
#         ":parquet",
#         "@com_google_googletest//:gtest_main",
#     ],
# )

cc_test(
    name = "file_deserialize_test",
    srcs = [
        "cpp/src/parquet/file_deserialize_test.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "schema_test",
    srcs = [
        "cpp/src/parquet/schema_test.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "column_io_benchmark",
    size = "large",
    srcs = [
        "cpp/src/parquet/column_io_benchmark.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)

# cc_test(
#     name = "encoding_benchmark",
#     size = "large",
#     srcs = [
#         "cpp/src/parquet/encoding_benchmark.cc",
#         "cpp/src/parquet/test_util.cc",
#     ],
#     deps = [
#         ":arrow_test_common",
#         ":parquet",
#         "@com_github_google_benchmark//:benchmark_main",
#     ],
# )

cc_test(
    name = "level_conversion_benchmark",
    size = "large",
    srcs = [
        "cpp/src/parquet/level_conversion_benchmark.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)

cc_test(
    name = "arrow_reader_writer_benchmark",
    size = "large",
    srcs = [
        "cpp/src/parquet/arrow/reader_writer_benchmark.cc",
        "cpp/src/parquet/test_util.cc",
    ],
    deps = [
        ":arrow_test_common",
        ":parquet",
        "@com_github_google_benchmark//:benchmark_main",
    ],
)
