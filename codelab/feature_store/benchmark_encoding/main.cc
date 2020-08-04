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
#include "tools/cpp/runfiles/runfiles.h"
#include "codelab/feature_store/benchmark_encoding/dump.h"
#include "codelab/feature_store/benchmark_encoding/feature.pb.h"
#include "codelab/feature_store/benchmark_encoding/row.h"
#include "codelab/pb_to_arrow/status_util.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/no_destructor.h"
#include "gtl/posix_file_system.h"

namespace {

constexpr const char kInputFile[] =
    "com_github_hcoona_one/codelab/feature_store/benchmark_encoding/"
    "rosetta_example_without_map_10.pb";
constexpr const size_t kDumpRowCount = 1000;

std::vector<codelab::feature_store::Row>* GetPreparedRows() {
  static gtl::NoDestructor<std::vector<codelab::feature_store::Row>> rows;
  return rows.get();
}

void BM_StringCopy(benchmark::State& state) {  // NOLINT
  std::string x = "hello";
  for (auto _ : state) std::string copy(x);
}

void BM_DumpWithParquetApi(benchmark::State& state) {  // NOLINT
  std::vector<codelab::feature_store::FieldDescriptor> fields;
  absl::Status s =
      codelab::feature_store::GenerateSchema(*GetPreparedRows(), &fields);
  CHECK(s.ok()) << s.ToString();

  std::vector<codelab::feature_store::Row> rows;
  rows.reserve(kDumpRowCount);
  for (size_t i = 0; i < kDumpRowCount; i++) {
    rows.emplace_back(
        GetPreparedRows()->operator[](i % GetPreparedRows()->size()));
  }

  int64_t proceeded_items = 0;
  int64_t written_bytes = 0;
  for (auto _ : state) {
    int64_t this_iter_written_bytes = 0;
    s = codelab::feature_store::DumpWithParquetApi(fields, rows,
                                                   &this_iter_written_bytes);
    CHECK(s.ok()) << s.ToString();

    proceeded_items += rows.size();
    state.SetItemsProcessed(proceeded_items);

    written_bytes += this_iter_written_bytes;
    state.SetBytesProcessed(written_bytes);
  }
}

void BM_DumpWithParquetApiV2(benchmark::State& state) {  // NOLINT
  std::vector<codelab::feature_store::FieldDescriptor> fields;
  absl::Status s =
      codelab::feature_store::GenerateSchema(*GetPreparedRows(), &fields);
  CHECK(s.ok()) << s.ToString();

  std::vector<codelab::feature_store::Row> rows;
  rows.reserve(kDumpRowCount);
  for (size_t i = 0; i < kDumpRowCount; i++) {
    rows.emplace_back(
        GetPreparedRows()->operator[](i % GetPreparedRows()->size()));
  }

  int64_t proceeded_items = 0;
  int64_t written_bytes = 0;
  for (auto _ : state) {
    int64_t this_iter_written_bytes = 0;
    s = codelab::feature_store::DumpWithParquetApiV2(fields, rows,
                                                     &this_iter_written_bytes);
    CHECK(s.ok()) << s.ToString();

    proceeded_items += rows.size();
    state.SetItemsProcessed(proceeded_items);

    written_bytes += this_iter_written_bytes;
    state.SetBytesProcessed(written_bytes);
  }
}

}  // namespace

BENCHMARK(BM_StringCopy);
BENCHMARK(BM_DumpWithParquetApi);
BENCHMARK(BM_DumpWithParquetApiV2);

DEFINE_string(input, "", "Input encoded protobuf data file.");

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<codelab::feature_store::Row>* rows);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  ::benchmark::Initialize(&argc, argv);

  std::string error_message;
  std::unique_ptr<bazel::tools::cpp::runfiles::Runfiles> runfiles(
      bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error_message));
  CHECK(runfiles) << "Failed to create Bazel runfiles context: "
                  << error_message;

  std::string input_file =
      FLAGS_input.empty() ? runfiles->Rlocation(kInputFile) : FLAGS_input;

  gtl::PosixFileSystem file_system;
  absl::Status s = LoadRows(&file_system, input_file, GetPreparedRows());
  CHECK(s.ok()) << "Failed to load rows: " << s.ToString();
  LOG(INFO) << GetPreparedRows()->size() << " rows loaded.";

  ::benchmark::RunSpecifiedBenchmarks();

  return 0;
}

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<codelab::feature_store::Row>* rows) {
  RETURN_STATUS_IF_NOT_OK(file_system->FileExists(input_file));

  std::unique_ptr<gtl::ReadOnlyMemoryRegion> memory_region;
  RETURN_STATUS_IF_NOT_OK(
      file_system->NewReadOnlyMemoryRegionFromFile(input_file, &memory_region));

  for (int offset = 0; offset < memory_region->length();) {
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

  return absl::OkStatus();
}
