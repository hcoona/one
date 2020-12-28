#include <memory>

#include "absl/base/casts.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "arrow/api.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "codelab/pb_to_arrow/converter.h"
#include "codelab/pb_to_arrow/messages.pb.h"
#include "codelab/pb_to_arrow/status_util.h"
#include "util/casts.h"

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  hcoona::codelab::Int32WrapperWrapper nested_int32_wrapper;
  google::protobuf::Message* message = &nested_int32_wrapper;
  absl::Span<const google::protobuf::Message* const> messages =
      absl::MakeConstSpan(
          absl::implicit_cast<google::protobuf::Message**>(&message), 1);

  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::Table> table;
  absl::Status s = hcoona::codelab::ConvertTable(*(message->GetDescriptor()),
                                                 messages, pool, &table);
  CHECK_STATUS_OK(s);
  s = hcoona::codelab::FromArrowStatus(table->ValidateFull());
  if (!s.ok()) {
    LOG(ERROR) << s.ToString();
  } else {
    LOG(WARNING) << "Converting bug fixed.";
  }

  // LOG(INFO) << "Schema: " << table->schema()->ToString();

  std::vector<std::shared_ptr<arrow::ArrayBuilder>> field_builders;
  field_builders.emplace_back(std::make_shared<arrow::Int32Builder>(pool));
  std::shared_ptr<arrow::StructBuilder> nested_int32_struct_builder =
      std::make_shared<arrow::StructBuilder>(
          arrow::struct_({arrow::field("int32_value", arrow::int32())}), pool,
          field_builders);
  arrow::Int32Builder* inner_int32_builder =
      hcoona::down_cast<arrow::Int32Builder*>(
          nested_int32_struct_builder->field_builder(0));
  CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      nested_int32_struct_builder->AppendNull()));
  CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(inner_int32_builder->AppendNull()));

  std::shared_ptr<arrow::Array> nested_int32_struct_array;
  CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      nested_int32_struct_builder->Finish(&nested_int32_struct_array)));
  table = arrow::Table::Make(
      arrow::schema({arrow::field(
          "value",
          arrow::struct_({arrow::field("int32_value", arrow::int32())}))}),
      {nested_int32_struct_array}, 1);
  LOG(INFO) << "Schema=" << table->schema()->ToString();

  s = hcoona::codelab::FromArrowStatus(table->ValidateFull());
  if (!s.ok()) {
    LOG(ERROR) << s.ToString();
  } else {
    LOG(INFO) << "Manual construction works.";
  }

  return 0;
}
