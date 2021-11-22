#include <future>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/internal/endian.h"
#include "absl/status/status.h"
#include "arrow/api.h"
#include "benchmark/benchmark.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "parquet/api/writer.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/no_destructor.h"
#include "gtl/posix_file_system.h"
#include "one/base/macros.h"
#include "one/codelab/feature_store/benchmark_encoding/dump.h"
#include "one/codelab/feature_store/benchmark_encoding/feature.pb.h"
#include "one/codelab/feature_store/benchmark_encoding/null_output_stream.h"
#include "one/codelab/feature_store/benchmark_encoding/row.h"

namespace {

constexpr const char kDefaultInputFile[] =
    "com_github_hcoona_one/one/codelab/feature_store/benchmark_encoding/"
    "rosetta_example_without_map_10.pb";
constexpr const size_t kDefaultDumpRowCount = 1000;

constexpr const char kReadBytesMetricName[] = "ReadBytes";
constexpr const char kWriteBytesMetricName[] = "WriteBytes";

}  // namespace

DEFINE_string(input, "", "Input encoded protobuf data file.");
DEFINE_uint32(dump_row_count, kDefaultDumpRowCount, "How many rows to dump.");
DEFINE_bool(input_standard_pack, false,
            "Whether the input file is packed with varint32 prefixed length "
            "delimited.");

namespace {

std::vector<hcoona::codelab::feature_store::Row>* GetPreparedRows() {
  static gtl::NoDestructor<std::vector<hcoona::codelab::feature_store::Row>>
      rows;
  return rows.get();
}

uint64_t* GetPreparedRowsBytes() {
  static gtl::NoDestructor<uint64_t> rows_bytes;
  return rows_bytes.get();
}

void BM_DumpWithParquetApi(benchmark::State& state) {  // NOLINT
  arrow::MemoryPool* memory_pool = arrow::default_memory_pool();

  std::vector<hcoona::codelab::feature_store::FieldDescriptor> fields;
  absl::Status s = hcoona::codelab::feature_store::GenerateSchema(
      *GetPreparedRows(), &fields);
  CHECK(s.ok()) << s.ToString();

  std::vector<hcoona::codelab::feature_store::Row> rows;
  uint64_t rows_bytes;
  if (FLAGS_dump_row_count == 0) {
    rows = *GetPreparedRows();
    rows_bytes = *GetPreparedRowsBytes();
  } else {
    rows.reserve(FLAGS_dump_row_count);
    for (size_t i = 0; i < FLAGS_dump_row_count; i++) {
      rows.emplace_back(
          GetPreparedRows()->operator[](i % GetPreparedRows()->size()));
    }
    rows_bytes =
        static_cast<uint64_t>(static_cast<double>(*GetPreparedRowsBytes()) /
                              GetPreparedRows()->size() * FLAGS_dump_row_count);
  }

  int64_t proceeded_items = 0;
  int64_t written_bytes = 0;
  uint64_t read_bytes = 0;
  for (auto _ : state) {
    auto output_stream =
        std::make_shared<hcoona::codelab::feature_store::NullOutputStream>();
    try {
      s = hcoona::codelab::feature_store::DumpWithParquetApi(
          memory_pool,
          static_cast<hcoona::codelab::feature_store::CompressionMode>(
              state.range(0)),
          fields, rows, output_stream);
      CHECK(s.ok()) << s.ToString();
    } catch (const parquet::ParquetException& e) {
      LOG(FATAL) << "Failed to dump with Parquet API: " << e.what();
    }

    proceeded_items += rows.size();
    written_bytes += output_stream->Tell().ValueOrDie();
    read_bytes += rows_bytes;
  }
  state.SetItemsProcessed(proceeded_items);
  state.counters[kReadBytesMetricName] =
      benchmark::Counter(read_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);
  state.counters[kWriteBytesMetricName] =
      benchmark::Counter(written_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);

  LOG(INFO) << "Items proceeded: " << proceeded_items;
  LOG(INFO) << "Written bytes: " << written_bytes;
  LOG(INFO) << "Average bytes per item(compressed): "
            << static_cast<double>(written_bytes) / proceeded_items;
}

void BM_DumpWithParquetApiV2(benchmark::State& state) {  // NOLINT
  arrow::MemoryPool* memory_pool = arrow::default_memory_pool();

  std::vector<hcoona::codelab::feature_store::FieldDescriptor> fields;
  absl::Status s = hcoona::codelab::feature_store::GenerateSchema(
      *GetPreparedRows(), &fields);
  CHECK(s.ok()) << s.ToString();

  std::vector<hcoona::codelab::feature_store::Row> rows;
  uint64_t rows_bytes;
  if (FLAGS_dump_row_count == 0) {
    rows = *GetPreparedRows();
    rows_bytes = *GetPreparedRowsBytes();
  } else {
    rows.reserve(FLAGS_dump_row_count);
    for (size_t i = 0; i < FLAGS_dump_row_count; i++) {
      rows.emplace_back(
          GetPreparedRows()->operator[](i % GetPreparedRows()->size()));
    }
    rows_bytes =
        static_cast<uint64_t>(static_cast<double>(*GetPreparedRowsBytes()) /
                              GetPreparedRows()->size() * FLAGS_dump_row_count);
  }

  int64_t proceeded_items = 0;
  int64_t written_bytes = 0;
  uint64_t read_bytes = 0;
  for (auto _ : state) {
    auto output_stream =
        std::make_shared<hcoona::codelab::feature_store::NullOutputStream>();
    s = hcoona::codelab::feature_store::DumpWithParquetApiV2(
        memory_pool,
        static_cast<hcoona::codelab::feature_store::CompressionMode>(
            state.range(0)),
        fields, rows, output_stream);
    CHECK(s.ok()) << s.ToString();

    proceeded_items += rows.size();
    written_bytes += output_stream->Tell().ValueOrDie();
    read_bytes += rows_bytes;
  }
  state.SetItemsProcessed(proceeded_items);
  state.counters[kReadBytesMetricName] =
      benchmark::Counter(read_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);
  state.counters[kWriteBytesMetricName] =
      benchmark::Counter(written_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);

  LOG(INFO) << "Items proceeded: " << proceeded_items;
  LOG(INFO) << "Written bytes: " << written_bytes;
  LOG(INFO) << "Average bytes per item(compressed): "
            << static_cast<double>(written_bytes) / proceeded_items;
}

void BM_DumpWithArrowApi(benchmark::State& state) {  // NOLINT
  arrow::MemoryPool* memory_pool = arrow::default_memory_pool();

  std::vector<hcoona::codelab::feature_store::FieldDescriptor> fields;
  absl::Status s = hcoona::codelab::feature_store::GenerateSchema(
      *GetPreparedRows(), &fields);
  CHECK(s.ok()) << s.ToString();

  std::vector<hcoona::codelab::feature_store::Row> rows;
  uint64_t rows_bytes;
  if (FLAGS_dump_row_count == 0) {
    rows = *GetPreparedRows();
    rows_bytes = *GetPreparedRowsBytes();
  } else {
    rows.reserve(FLAGS_dump_row_count);
    for (size_t i = 0; i < FLAGS_dump_row_count; i++) {
      rows.emplace_back(
          GetPreparedRows()->operator[](i % GetPreparedRows()->size()));
    }
    rows_bytes =
        static_cast<uint64_t>(static_cast<double>(*GetPreparedRowsBytes()) /
                              GetPreparedRows()->size() * FLAGS_dump_row_count);
  }

  int64_t proceeded_items = 0;
  int64_t written_bytes = 0;
  uint64_t read_bytes = 0;
  for (auto _ : state) {
    auto output_stream =
        std::make_shared<hcoona::codelab::feature_store::NullOutputStream>();
    s = hcoona::codelab::feature_store::DumpWithArrowApi(
        memory_pool,
        static_cast<hcoona::codelab::feature_store::CompressionMode>(
            state.range(0)),
        fields, rows, output_stream);
    CHECK(s.ok()) << s.ToString();

    proceeded_items += rows.size();
    written_bytes += output_stream->Tell().ValueOrDie();
    read_bytes += rows_bytes;
  }
  state.SetItemsProcessed(proceeded_items);
  state.counters[kReadBytesMetricName] =
      benchmark::Counter(read_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);
  state.counters[kWriteBytesMetricName] =
      benchmark::Counter(written_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);

  LOG(INFO) << "Items proceeded: " << proceeded_items;
  LOG(INFO) << "Written bytes: " << written_bytes;
  LOG(INFO) << "Average bytes per item(compressed): "
            << static_cast<double>(written_bytes) / proceeded_items;
}

}  // namespace

BENCHMARK(BM_DumpWithParquetApi)->Arg(0)->Arg(1)->ArgName("CompressionMode");
BENCHMARK(BM_DumpWithParquetApiV2)->Arg(0)->Arg(1)->ArgName("CompressionMode");
BENCHMARK(BM_DumpWithArrowApi)->Arg(0)->Arg(1)->ArgName("CompressionMode");

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<hcoona::codelab::feature_store::Row>* rows,
                      uint64_t* rows_bytes);

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::benchmark::Initialize(&argc, argv);

  std::string error_message;
  std::unique_ptr<bazel::tools::cpp::runfiles::Runfiles> runfiles(
      bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error_message));
  CHECK(runfiles) << "Failed to create Bazel runfiles context: "
                  << error_message;

  std::string input_file = FLAGS_input.empty()
                               ? runfiles->Rlocation(kDefaultInputFile)
                               : FLAGS_input;

  gtl::PosixFileSystem file_system;
  LOG(INFO) << "Loading input file " << input_file;
  absl::Status s = LoadRows(&file_system, input_file, GetPreparedRows(),
                            GetPreparedRowsBytes());
  CHECK(s.ok()) << "Failed to load rows: " << s.ToString();
  LOG(INFO) << GetPreparedRows()->size() << " rows(" << *GetPreparedRowsBytes()
            << " bytes) loaded.";

  ::benchmark::RunSpecifiedBenchmarks();

  return 0;
}

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<hcoona::codelab::feature_store::Row>* rows,
                      uint64_t* rows_bytes) {
  ONE_RETURN_STATUS_IF_NOT_OK(file_system->FileExists(input_file));

  std::unique_ptr<gtl::ReadOnlyMemoryRegion> memory_region;
  ONE_RETURN_STATUS_IF_NOT_OK(
      file_system->NewReadOnlyMemoryRegionFromFile(input_file, &memory_region));

  std::vector<std::future<idl::euclid::common::Example>> futures;

  if (FLAGS_input_standard_pack) {
    google::protobuf::io::ArrayInputStream array_input_stream(
        memory_region->data(), memory_region->length());
    google::protobuf::io::CodedInputStream coded_input_stream(
        &array_input_stream);

    while (static_cast<uint64_t>(coded_input_stream.CurrentPosition()) !=
           memory_region->length()) {
      uint32_t message_size;
      if (!coded_input_stream.ReadVarint32(&message_size)) {
        return absl::UnknownError("Failed to read varint32 from input file.");
      }

      const void* data;
      int size = message_size;
      if (!coded_input_stream.GetDirectBufferPointer(&data, &size)) {
        return absl::UnknownError(
            "Failed to read message from input file (GetDirectBufferPointer).");
      }
      if (size < message_size) {
        return absl::UnknownError(
            "GetDirectBufferPointer returned size mismatch required message "
            "size.");
      }
      CHECK_GT(message_size, 0);

      futures.emplace_back(std::async(
          std::launch::async,
          [](const void* data, int size) {
            idl::euclid::common::Example example;
            CHECK(example.ParseFromArray(data, size))
                << "Failed to parse idl::euclid::common::Example.";
            return example;
          },
          data, message_size));

      if (!coded_input_stream.Skip(message_size)) {
        return absl::UnknownError(
            "Failed to read message from input file (No enough data).");
      }
    }
  } else {
    for (int offset = 0; offset < memory_region->length();) {
      uint64_t message_size = absl::big_endian::Load64(
          reinterpret_cast<const uint8_t*>(memory_region->data()) + offset);
      offset += sizeof(message_size);

      futures.emplace_back(std::async(
          std::launch::async,
          [](const void* data, uint64_t size) {
            idl::euclid::common::Example example;
            CHECK(example.ParseFromArray(data, size))
                << "Failed to parse idl::euclid::common::Example.";
            return example;
          },
          reinterpret_cast<const uint8_t*>(memory_region->data()) + offset,
          message_size));
      offset += message_size;
    }
  }

  int counter = 0;
  uint64_t total_size = 0;
  for (std::future<idl::euclid::common::Example>& future : futures) {
    if (counter % 100 == 0) {
      LOG(INFO) << "Loading " << counter << "th row...";
    }
    if (FLAGS_dump_row_count != 0 && counter == FLAGS_dump_row_count) {
      break;
    }

    future.wait();
    idl::euclid::common::Example example = future.get();
    total_size += example.ByteSize();
    rows->emplace_back(std::move(example));

    counter++;
  }

  *rows_bytes = total_size;

  return absl::OkStatus();
}
