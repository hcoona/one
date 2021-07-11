#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gflags/gflags.h"
#include "third_party/glog/logging.h"
#include "third_party/absl/base/internal/endian.h"
#include "third_party/absl/status/status.h"
#include "third_party/arrow/src/arrow/api.h"
#include "third_party/arrow/src/arrow/io/api.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/no_destructor.h"
#include "gtl/posix_file_system.h"
#include "one/base/macros.h"
#include "one/codelab/feature_store/benchmark_encoding/dump.h"
#include "one/codelab/feature_store/benchmark_encoding/feature.pb.h"
#include "one/codelab/feature_store/benchmark_encoding/row.h"

namespace {

constexpr const char kDefaultInputFile[] =
    "com_github_hcoona_one/codelab/feature_store/benchmark_encoding/"
    "rosetta_example_without_map_10.pb";
constexpr const size_t kDefaultDumpRowCount = 1000;

bool FlagStringNotEmpty(const char* flag_name, const std::string& value) {
  ignore_result(flag_name);
  return !value.empty();
}

std::vector<hcoona::codelab::feature_store::Row>* GetPreparedRows() {
  static gtl::NoDestructor<std::vector<hcoona::codelab::feature_store::Row>>
      rows;
  return rows.get();
}

}  // namespace

DEFINE_string(input, "", "Input encoded protobuf data file.");
DEFINE_string(output, "", "Output parquet file directory.");
DEFINE_validator(output, &FlagStringNotEmpty);
DEFINE_bool(compress, false, "Whether to compress the output files.");

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<hcoona::codelab::feature_store::Row>* rows);

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);

  std::string error_message;
  std::unique_ptr<bazel::tools::cpp::runfiles::Runfiles> runfiles(
      bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error_message));
  CHECK(runfiles) << "Failed to create Bazel runfiles context: "
                  << error_message;

  std::string input_file = FLAGS_input.empty()
                               ? runfiles->Rlocation(kDefaultInputFile)
                               : FLAGS_input;

  gtl::PosixFileSystem file_system;
  absl::Status s = file_system.IsDirectory(FLAGS_output);
  if (absl::IsNotFound(s)) {
    s = file_system.CreateDir(FLAGS_output);
    CHECK(s.ok()) << "Failed to create output directory '" << FLAGS_output
                  << "': " << s.ToString();
  } else {
    CHECK(s.ok()) << "Output directory error: " << s.ToString();
  }

  s = LoadRows(&file_system, input_file, GetPreparedRows());
  CHECK(s.ok()) << "Failed to load rows: " << s.ToString();
  LOG(INFO) << GetPreparedRows()->size() << " rows loaded.";

  arrow::MemoryPool* memory_pool = arrow::default_memory_pool();

  std::vector<hcoona::codelab::feature_store::FieldDescriptor> fields;
  s = hcoona::codelab::feature_store::GenerateSchema(*GetPreparedRows(),
                                                     &fields);
  CHECK(s.ok()) << s.ToString();

  std::vector<hcoona::codelab::feature_store::Row> rows;
  rows.reserve(kDefaultDumpRowCount);
  for (size_t i = 0; i < kDefaultDumpRowCount; i++) {
    rows.emplace_back(
        GetPreparedRows()->operator[](i % GetPreparedRows()->size()));
  }

  std::shared_ptr<arrow::io::OutputStream> sink1 =
      arrow::io::FileOutputStream::Open(
          file_system.JoinPath(FLAGS_output, "parquet_api_v1.parquet"))
          .ValueOrDie();
  s = hcoona::codelab::feature_store::DumpWithParquetApi(
      memory_pool,
      hcoona::codelab::feature_store::CompressionMode::kNoCompression, fields,
      rows, sink1);
  CHECK(s.ok()) << s.ToString();

  std::shared_ptr<arrow::io::OutputStream> sink2 =
      arrow::io::FileOutputStream::Open(
          file_system.JoinPath(FLAGS_output, "parquet_api_v2.parquet"))
          .ValueOrDie();
  s = hcoona::codelab::feature_store::DumpWithParquetApiV2(
      memory_pool,
      hcoona::codelab::feature_store::CompressionMode::kNoCompression, fields,
      rows, sink2);
  CHECK(s.ok()) << s.ToString();

  std::shared_ptr<arrow::io::OutputStream> sink3 =
      arrow::io::FileOutputStream::Open(
          file_system.JoinPath(FLAGS_output, "arrow_api.parquet"))
          .ValueOrDie();
  s = hcoona::codelab::feature_store::DumpWithArrowApi(
      memory_pool,
      hcoona::codelab::feature_store::CompressionMode::kNoCompression, fields,
      rows, sink3);
  CHECK(s.ok()) << s.ToString();

  return 0;
}

absl::Status LoadRows(gtl::FileSystem* file_system,
                      const std::string& input_file,
                      std::vector<hcoona::codelab::feature_store::Row>* rows) {
  ONE_RETURN_STATUS_IF_NOT_OK(file_system->FileExists(input_file));

  std::unique_ptr<gtl::ReadOnlyMemoryRegion> memory_region;
  ONE_RETURN_STATUS_IF_NOT_OK(
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
