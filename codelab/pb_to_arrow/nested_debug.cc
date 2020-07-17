#include <memory>

#include "absl/base/casts.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "arrow/api.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "codelab/pb_to_arrow/converter.h"
#include "codelab/pb_to_arrow/messages.pb.h"
#include "codelab/pb_to_arrow/status_util.h"
#include "util/casts.h"

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  hcoona::codelab::DoubleListWrapper double_list_wrapper;
  double_list_wrapper.add_double_list(3);
  double_list_wrapper.add_double_list(5);
  google::protobuf::Message* message = &double_list_wrapper;
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

  LOG(INFO) << "Schema: " << table->schema()->ToString();

  std::shared_ptr<arrow::ListBuilder> double_list_builder =
      std::make_shared<arrow::ListBuilder>(
          pool, std::make_shared<arrow::DoubleBuilder>(pool),
          arrow::list(arrow::float64()));
  arrow::DoubleBuilder* double_list_value_builder =
      hcoona::down_cast<arrow::DoubleBuilder*>(
          double_list_builder->value_builder());
  CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(double_list_builder->Append()));
  CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(double_list_value_builder->Append(3)));
  CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(double_list_value_builder->Append(5)));

  std::shared_ptr<arrow::Array> double_list_array;
  CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      double_list_builder->Finish(&double_list_array)));
  table = arrow::Table::Make(table->schema(), {double_list_array}, 1);

  s = hcoona::codelab::FromArrowStatus(table->ValidateFull());
  if (!s.ok()) {
    LOG(ERROR) << s.ToString();
  } else {
    LOG(INFO) << "Manual construction works.";
  }

  return 0;
}
