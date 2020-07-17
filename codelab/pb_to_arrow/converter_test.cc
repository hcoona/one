#include "codelab/pb_to_arrow/converter.h"

#include <memory>

#include "absl/base/casts.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "arrow/api.h"
#include "gtest/gtest.h"
#include "codelab/pb_to_arrow/messages.pb.h"
#include "codelab/pb_to_arrow/status_util.h"

namespace hcoona {
namespace codelab {
namespace {

TEST(TestDouble, TestDoubleValue) {
  DoubleWrapper double_wrapper;
  double_wrapper.set_double_value(3);
  google::protobuf::Message* message = &double_wrapper;
  absl::Span<const google::protobuf::Message* const> messages =
      absl::MakeConstSpan(
          absl::implicit_cast<google::protobuf::Message**>(&message), 1);

  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::Table> table;
  absl::Status s =
      ConvertTable(*(message->GetDescriptor()), messages, pool, &table);
  ASSERT_TRUE(s.ok()) << s.ToString();

  s = FromArrowStatus(table->ValidateFull());
  EXPECT_TRUE(s.ok()) << s.ToString();
}

TEST(TestDouble, TestDoubleList) {
  DoubleListWrapper double_list_wrapper;
  double_list_wrapper.add_double_list(3);
  double_list_wrapper.add_double_list(5);
  google::protobuf::Message* message = &double_list_wrapper;
  absl::Span<const google::protobuf::Message* const> messages =
      absl::MakeConstSpan(
          absl::implicit_cast<google::protobuf::Message**>(&message), 1);

  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::Table> table;
  absl::Status s =
      ConvertTable(*(message->GetDescriptor()), messages, pool, &table);
  ASSERT_TRUE(s.ok()) << s.ToString();

  s = FromArrowStatus(table->ValidateFull());
  EXPECT_TRUE(s.ok()) << s.ToString();
}

}  // namespace
}  // namespace codelab
}  // namespace hcoona
