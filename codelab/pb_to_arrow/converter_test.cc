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
  ClearArrowArrayBuilderDebugContext();

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
  ClearArrowArrayBuilderDebugContext();

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

TEST(TestDouble, TestDoubleMap) {
  ClearArrowArrayBuilderDebugContext();

  DoubleMapWrapper double_map_wrapper;
  double_map_wrapper.mutable_double_map()->operator[](3) = 3;
  double_map_wrapper.mutable_double_map()->operator[](5) = 5;
  google::protobuf::Message* message = &double_map_wrapper;
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

TEST(TestInt32, TestInt32Value) {
  ClearArrowArrayBuilderDebugContext();

  Int32Wrapper int32_wrapper;
  int32_wrapper.set_int32_value(3);
  google::protobuf::Message* message = &int32_wrapper;
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

TEST(TestInt32, TestInt32List) {
  ClearArrowArrayBuilderDebugContext();

  Int32ListWrapper int32_list_wrapper;
  int32_list_wrapper.add_int32_list(3);
  int32_list_wrapper.add_int32_list(5);
  google::protobuf::Message* message = &int32_list_wrapper;
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

TEST(TestInt32, TestInt32Map) {
  ClearArrowArrayBuilderDebugContext();

  Int32MapWrapper int32_map_wrapper;
  int32_map_wrapper.mutable_int32_map()->operator[](3) = 3;
  int32_map_wrapper.mutable_int32_map()->operator[](5) = 5;
  google::protobuf::Message* message = &int32_map_wrapper;
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

TEST(TestNest, TestNestInt32Value) {
  ClearArrowArrayBuilderDebugContext();

  NestInt32Wrapper nest_int32_wrapper;
  nest_int32_wrapper.mutable_value()->set_int32_value(3);
  google::protobuf::Message* message = &nest_int32_wrapper;
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
