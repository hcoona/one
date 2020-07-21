#include <memory>
#include <string>
#include <utility>

#include "absl/base/casts.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "arrow/api.h"
#include "arrow/io/api.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "codelab/pb_to_arrow/converter.h"
#include "codelab/pb_to_arrow/messages.pb.h"
#include "codelab/pb_to_arrow/status_util.h"

namespace {

constexpr const size_t kMessageCount = 10;
constexpr const char kMyString[] = "Hello World!";

bool FlagStringNotEmpty(const char* flag_name, const std::string& value) {
  ignore_result(flag_name);
  return !value.empty();
}

}  // namespace

DEFINE_string(output, "arrow_test.parquet", "Output file.");
DEFINE_validator(output, &FlagStringNotEmpty);

void FillMessageA(
    std::vector<std::shared_ptr<hcoona::codelab::MessageA>>* messages);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::vector<std::shared_ptr<hcoona::codelab::MessageA>> message_vector;
  FillMessageA(&message_vector);

  std::vector<const google::protobuf::Message*> messages;
  messages.reserve(message_vector.size());
  for (const std::shared_ptr<hcoona::codelab::MessageA>& m : message_vector) {
    DLOG(INFO) << m->DebugString();
    messages.emplace_back(m.get());
  }

  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::Table> table;
  CHECK_STATUS_OK(hcoona::codelab::ConvertTable(
      *hcoona::codelab::MessageA::GetDescriptor(),
      absl::MakeConstSpan(messages), pool, &table));
  LOG(INFO) << "Table: " << table->ToString();
  CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(table->ValidateFull()));

  std::shared_ptr<arrow::io::FileOutputStream> output_file =
      arrow::io::FileOutputStream::Open(FLAGS_output).ValueOrDie();
  CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      parquet::arrow::WriteTable(*table, pool, output_file, 128)));
  LOG(INFO) << kMessageCount << " messages written to " << FLAGS_output;

  return 0;
}

void FillMessageA(
    std::vector<std::shared_ptr<hcoona::codelab::MessageA>>* messages) {
  DCHECK_NOTNULL(messages);
  absl::BitGen bitgen;

  for (int i = 0; i < kMessageCount; i++) {
    auto m = std::make_unique<hcoona::codelab::MessageA>();
    m->set_my_int32_value(i);
    m->set_my_int64_value(kMessageCount - i - 1);
    m->set_my_uint64_value(absl::Uniform(bitgen,
                                         std::numeric_limits<uint64_t>::min(),
                                         std::numeric_limits<uint64_t>::max()));
    m->set_my_bool_value(i % 2 == 0);
    m->set_my_string_value(kMyString);

    int s = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
    for (int j = 0; j < s; j++) {
      m->add_id(j);
    }

    s = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
    for (int j = 0; j < s; j++) {
      m->mutable_my_map()->operator[](j) = absl::Zipf<int32_t>(bitgen);
    }

    s = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
    for (int j = 0; j < s; j++) {
      hcoona::codelab::MessageA_NestedMessageB* b = m->add_my_message_b_value();
      int t = std::max(0, static_cast<int>(absl::Gaussian(bitgen, 5.0)));
      for (int k = 0; k < t; k++) {
        b->add_my_sfixed64_value(absl::Zipf<int64_t>(bitgen));
      }
    }

    if (i % 2 == 0) {
      m->set_my_oneof_string_value(kMyString);
    } else {
      m->set_my_oneof_int64_value(i);
    }

    messages->emplace_back(std::move(m));
  }
}
