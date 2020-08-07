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
#include "tools/cpp/runfiles/runfiles.h"
#include "codelab/feature_store/benchmark_encoding/dump.h"
#include "codelab/feature_store/benchmark_encoding/feature.pb.h"
#include "codelab/feature_store/benchmark_encoding/null_output_stream.h"
#include "codelab/feature_store/benchmark_encoding/row.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/no_destructor.h"
#include "gtl/posix_file_system.h"
#include "status/status_util.h"

namespace {

constexpr const char kDefaultInputFile[] =
    "com_github_hcoona_one/codelab/feature_store/benchmark_encoding/"
    "rosetta_example_without_map_10.pb";
constexpr const size_t kDefaultDumpRowCount = 1000;

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
    s = hcoona::codelab::feature_store::DumpWithParquetApi(
        memory_pool,
        static_cast<hcoona::codelab::feature_store::CompressionMode>(
            state.range(0)),
        fields, rows, output_stream);
    CHECK(s.ok()) << s.ToString();

    proceeded_items += rows.size();
    written_bytes += output_stream->Tell().ValueOrDie();
    read_bytes += rows_bytes;
  }
  state.counters["proceeded_rows_per_second"] =
      benchmark::Counter(proceeded_items, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1000);
  state.counters["written_bytes_per_second"] =
      benchmark::Counter(written_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);
  state.counters["read_bytes_per_second"] =
      benchmark::Counter(read_bytes, benchmark::Counter::Flags::kIsRate,
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
  state.counters["proceeded_rows_per_second"] =
      benchmark::Counter(proceeded_items, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1000);
  state.counters["written_bytes_per_second"] =
      benchmark::Counter(written_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);
  state.counters["read_bytes_per_second"] =
      benchmark::Counter(read_bytes, benchmark::Counter::Flags::kIsRate,
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
  state.counters["proceeded_rows_per_second"] =
      benchmark::Counter(proceeded_items, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1000);
  state.counters["written_bytes_per_second"] =
      benchmark::Counter(written_bytes, benchmark::Counter::Flags::kIsRate,
                         benchmark::Counter::OneK::kIs1024);
  state.counters["read_bytes_per_second"] =
      benchmark::Counter(read_bytes, benchmark::Counter::Flags::kIsRate,
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
  LOG(INFO) << GetPreparedRows()->size() << " rows loaded.";

  ::benchmark::RunSpecifiedBenchmarks();

  return 0;
}

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<hcoona::codelab::feature_store::Row>* rows,
                      uint64_t* rows_bytes) {
  RETURN_STATUS_IF_NOT_OK(file_system->FileExists(input_file));

  std::unique_ptr<gtl::ReadOnlyMemoryRegion> memory_region;
  RETURN_STATUS_IF_NOT_OK(
      file_system->NewReadOnlyMemoryRegionFromFile(input_file, &memory_region));

  uint64_t counter = 0;
  if (FLAGS_input_standard_pack) {
    google::protobuf::io::ArrayInputStream array_input_stream(
        memory_region->data(), memory_region->length());
    google::protobuf::io::CodedInputStream coded_input_stream(
        &array_input_stream);

    bool clean_eof;
    idl::euclid::common::Example example;
    while (true) {
      if (counter++ % 100 == 0) {
        LOG(INFO) << "Loading " << counter - 1 << "th row...";
      }
      bool succeeded = google::protobuf::util::ParseDelimitedFromCodedStream(
          &example, &coded_input_stream, &clean_eof);
      if (succeeded) {
        rows->emplace_back(std::move(example));
      } else {
        if (clean_eof) {
          break;
        } else {
          return absl::UnknownError("Failed to parse message from input file.");
        }
      }
    }
  } else {
    for (int offset = 0; offset < memory_region->length();) {
      if (counter++ % 100 == 0) {
        LOG(INFO) << "Loading " << counter - 1 << "th row...";
      }

      uint64_t message_size = absl::big_endian::Load64(
          reinterpret_cast<const uint8_t*>(memory_region->data()) + offset);
      offset += sizeof(message_size);

      idl::euclid::common::Example example;
      CHECK(example.ParseFromArray(
          reinterpret_cast<const uint8_t*>(memory_region->data()) + offset,
          message_size))
          << "Failed to parse example, offset=" << offset
          << ", message_size=" << message_size;
      offset += message_size;

      rows->emplace_back(std::move(example));
    }
  }

  *rows_bytes = memory_region->length();

  return absl::OkStatus();
}
