#include <memory>
#include <string>
#include <utility>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "third_party/absl/base/casts.h"
#include "third_party/absl/random/random.h"
#include "third_party/absl/status/status.h"
#include "third_party/absl/types/span.h"
#include "third_party/arrow/src/arrow/api.h"
#include "third_party/arrow/src/arrow/io/api.h"
#include "third_party/arrow/src/parquet/arrow/reader.h"
#include "third_party/arrow/src/parquet/arrow/writer.h"
#include "one/base/macros.h"
#include "one/codelab/pb_to_arrow/converter.h"
#include "one/codelab/pb_to_arrow/messages.pb.h"
#include "one/codelab/pb_to_arrow/status_util.h"

namespace {

constexpr const size_t kMessageCount = 10;
constexpr const char kMyString[] = "Hello World!";

bool FlagStringNotEmpty(const char* /*flag_name*/, const std::string& value) {
  return !value.empty();
}

}  // namespace

DEFINE_string(output, "arrow_test.parquet", "Output file.");
DEFINE_validator(output, &FlagStringNotEmpty);

void FillMessageX(
    std::vector<std::shared_ptr<hcoona::codelab::MessageX>>* messages);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::vector<std::shared_ptr<hcoona::codelab::MessageX>> message_vector;
  FillMessageX(&message_vector);

  std::vector<const google::protobuf::Message*> messages;
  messages.reserve(message_vector.size());
  for (const std::shared_ptr<hcoona::codelab::MessageX>& m : message_vector) {
    DLOG(INFO) << m->DebugString();
    messages.emplace_back(m.get());
  }

  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::Table> table;
  ONE_CHECK_STATUS_OK(hcoona::codelab::ConvertTable(
      *hcoona::codelab::MessageX::GetDescriptor(),
      absl::MakeConstSpan(messages), pool, &table));
  LOG(INFO) << "Table: " << table->ToString();
  ONE_CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(table->ValidateFull()));

  std::shared_ptr<arrow::io::FileOutputStream> output_file =
      arrow::io::FileOutputStream::Open(FLAGS_output).ValueOrDie();
  ONE_CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      parquet::arrow::WriteTable(*table, pool, output_file, 128)));
  LOG(INFO) << kMessageCount << " messages written to " << FLAGS_output;

  std::shared_ptr<arrow::io::MemoryMappedFile> input_file =
      arrow::io::MemoryMappedFile::Open(FLAGS_output, arrow::io::FileMode::READ)
          .ValueOrDie();

  parquet::arrow::FileReaderBuilder file_reader_builder;
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(file_reader_builder.Open(input_file)));

  std::unique_ptr<parquet::arrow::FileReader> file_reader;
  ONE_CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      file_reader_builder.Build(&file_reader)));

  std::vector<int> column_indices = {1, 8};
  // std::shared_ptr<arrow::Table> table;
  table.reset();
  ONE_CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      file_reader->ReadTable(column_indices, &table)));
  LOG(INFO) << table->ToString();

  return 0;
}

void FillMessageX(
    std::vector<std::shared_ptr<hcoona::codelab::MessageX>>* messages) {
  DCHECK_NOTNULL(messages);
  absl::BitGen bitgen;

  // for (int i = 0; i < kMessageCount; i++) {
  //   auto m = std::make_unique<hcoona::codelab::MessageX>();
  //   m->set_my_int32_value(i);
  //   m->set_my_int64_value(kMessageCount - i - 1);
  //   m->set_my_bool_value(i % 2 == 0);
  //   m->set_my_string_value(kMyString);

  //   int s = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
  //   for (int j = 0; j < s; j++) {
  //     m->add_id(j);
  //   }

  //   s = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
  //   for (int j = 0; j < s; j++) {
  //     m->mutable_my_map()->operator[](j) = absl::Zipf<int32_t>(bitgen);
  //   }

  //   s = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
  //   for (int j = 0; j < s; j++) {
  //     hcoona::codelab::MessageX_NestedMessageB* b =
  //     m->add_my_message_b_value(); int t = std::max(0,
  //     static_cast<int>(absl::Gaussian(bitgen, 5.0))); for (int k = 0; k < t;
  //     k++) {
  //       b->add_my_sfixed64_value(absl::Zipf<int64_t>(bitgen));
  //     }
  //   }

  //   if (i % 2 == 0) {
  //     m->set_my_oneof_string_value(kMyString);
  //   } else {
  //     m->set_my_oneof_int64_value(i);
  //   }

  //   messages->emplace_back(std::move(m));
  // }

  auto m = std::make_unique<hcoona::codelab::MessageX>();

  m->mutable_my_map()->operator[]("1") = 1;
  m->mutable_my_map()->operator[]("2") = 3;

  messages->emplace_back(std::move(m));
}
