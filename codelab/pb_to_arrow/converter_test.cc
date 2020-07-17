#include "codelab/pb_to_arrow/converter.h"

#include <memory>
#include <utility>

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
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&double_wrapper);
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
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&double_list_wrapper);
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
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&double_map_wrapper);
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
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&int32_wrapper);
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
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&int32_list_wrapper);
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
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&int32_map_wrapper);
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

TEST(TestNest, TestInt32WrapperWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32WrapperWrapper int32_wrapper_wrapper;
  int32_wrapper_wrapper.mutable_value()->set_int32_value(3);
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(&int32_wrapper_wrapper);
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

TEST(TestNest, Int32WrapperListWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32WrapperListWrapper int32_wrapper_list_wrapper;
  int32_wrapper_list_wrapper.add_value()->set_int32_value(3);
  int32_wrapper_list_wrapper.add_value()->set_int32_value(5);
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(
          &int32_wrapper_list_wrapper);
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

TEST(TestNest, EmptyInt32ListWrapperListWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32ListWrapperListWrapper int32_list_wrapper_list_wrapper;

  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(
          &int32_list_wrapper_list_wrapper);
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

TEST(TestNest, Int32ListWrapperListWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32ListWrapperListWrapper int32_list_wrapper_list_wrapper;
  Int32ListWrapper* value1 = int32_list_wrapper_list_wrapper.add_value();
  value1->add_int32_list(3);
  value1->add_int32_list(5);
  Int32ListWrapper* value2 = int32_list_wrapper_list_wrapper.add_value();
  value2->add_int32_list(7);
  value2->add_int32_list(11);
  value2->add_int32_list(13);

  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(
          &int32_list_wrapper_list_wrapper);
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

TEST(TestNest, Int32WrapperMapWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32WrapperMapWrapper int32_wrapper_map_wrapper;
  {
    Int32Wrapper value3;
    value3.set_int32_value(3);
    int32_wrapper_map_wrapper.mutable_value()->operator[](3) =
        std::move(value3);
  }
  {
    Int32Wrapper value5;
    value5.set_int32_value(5);
    int32_wrapper_map_wrapper.mutable_value()->operator[](5) =
        std::move(value5);
  }
  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(
          &int32_wrapper_map_wrapper);
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
