#include "one/codelab/pb_to_arrow/converter.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "third_party/absl/base/casts.h"
#include "third_party/absl/status/status.h"
#include "third_party/absl/types/span.h"
#include "third_party/arrow/src/arrow/api.h"
#include "third_party/glog/logging.h"
#include "one/codelab/pb_to_arrow/messages.pb.h"
#include "one/codelab/pb_to_arrow/status_util.h"

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
  int32_list_wrapper.add_int32_list(2);
  int32_list_wrapper.add_int32_list(3);
  int32_list_wrapper.add_int32_list(5);
  int32_list_wrapper.add_int32_list(7);
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

TEST(TestNest, TestInt32WrapperListWrapper) {
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

TEST(TestNest, TestEmptyInt32ListWrapperListWrapper) {
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

TEST(TestNest, TestInt32ListWrapperListWrapper) {
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

TEST(TestNest, TestInt32WrapperMapWrapper) {
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

TEST(TestNest, TestInt32ListWrapperWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32ListWrapperWrapper int32_list_wrapper_wrapper;
  int32_list_wrapper_wrapper.mutable_value()->add_int32_list(2);
  int32_list_wrapper_wrapper.mutable_value()->add_int32_list(3);
  int32_list_wrapper_wrapper.mutable_value()->add_int32_list(5);
  int32_list_wrapper_wrapper.mutable_value()->add_int32_list(7);

  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(
          &int32_list_wrapper_wrapper);
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

TEST(TestNest, TestEmptyInt32ListWrapperWrapper) {
  ClearArrowArrayBuilderDebugContext();

  Int32ListWrapperWrapper int32_list_wrapper_wrapper;

  google::protobuf::Message* message =
      absl::implicit_cast<google::protobuf::Message*>(
          &int32_list_wrapper_wrapper);
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

TEST(TestComplexData, TestEmptyMessageA) {
  ClearArrowArrayBuilderDebugContext();

  MessageA message_a;

  google::protobuf::Message* message = &message_a;
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

TEST(TestComplexData, TestMessageACase1) {
  ClearArrowArrayBuilderDebugContext();

  MessageA message_a;
  message_a.add_id(2);
  message_a.add_id(3);
  message_a.add_id(5);
  message_a.add_id(7);

  google::protobuf::Message* message = &message_a;
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

TEST(TestComplexData, TestMessageACase2) {
  ClearArrowArrayBuilderDebugContext();

  constexpr const double PI = 3.14159265358979323846;
  constexpr const char HELLO_WORLD[] = "Hello World!";

  MessageA message_a;
  message_a.set_my_int32_value(2);
  message_a.set_my_int64_value(3);
  message_a.set_my_uint64_value(5);
  message_a.set_my_bool_value(true);
  message_a.set_my_double_value(PI);
  message_a.set_my_string_value(HELLO_WORLD);
  message_a.set_my_bytes_value(HELLO_WORLD);
  message_a.set_my_enum_value(EnumA::ENUMA_Y);
  message_a.add_id(7);
  message_a.add_id(11);
  message_a.add_id(13);
  message_a.add_id(17);
  message_a.mutable_my_map()->insert(
      google::protobuf::MapPair<int32_t, int32_t>(1, 2));
  message_a.mutable_my_map()->insert(
      google::protobuf::MapPair<int32_t, int32_t>(2, 3));
  message_a.mutable_my_map()->insert(
      google::protobuf::MapPair<int32_t, int32_t>(3, 5));
  message_a.mutable_my_map()->insert(
      google::protobuf::MapPair<int32_t, int32_t>(4, 8));
  message_a.mutable_my_map()->insert(
      google::protobuf::MapPair<int32_t, int32_t>(5, 13));
  message_a.set_my_oneof_string_value(HELLO_WORLD);

  google::protobuf::Message* message = &message_a;
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

  LOG(INFO) << table->ToString();
}

}  // namespace
}  // namespace codelab
}  // namespace hcoona
