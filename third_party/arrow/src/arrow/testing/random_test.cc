// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
#include "third_party/googletest/googletest/include/gtest/gtest.h"

#include "third_party/arrow/src/arrow/array.h"
#include "third_party/arrow/src/arrow/record_batch.h"
#include "third_party/arrow/src/arrow/testing/gtest_util.h"
#include "third_party/arrow/src/arrow/testing/random.h"
#include "third_party/arrow/src/arrow/type.h"
#include "third_party/arrow/src/arrow/util/checked_cast.h"
#include "third_party/arrow/src/arrow/util/key_value_metadata.h"

namespace arrow {

using internal::checked_cast;

namespace random {

// Use short arrays since especially in debug mode, generating list(list()) is slow
constexpr int64_t kExpectedLength = 24;

class RandomArrayTest : public ::testing::TestWithParam<std::shared_ptr<Field>> {
 protected:
  std::shared_ptr<Field> GetField() { return GetParam(); }
};

template <typename T>
class RandomNumericArrayTest : public ::testing::Test {
 protected:
  std::shared_ptr<Field> GetField() { return field("field0", std::make_shared<T>()); }

  std::shared_ptr<NumericArray<T>> Downcast(std::shared_ptr<Array> array) {
    return internal::checked_pointer_cast<NumericArray<T>>(array);
  }
};

TEST_P(RandomArrayTest, GenerateArray) {
  auto field = GetField();
  auto array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), array->type());
  ASSERT_EQ(kExpectedLength, array->length());
  ASSERT_OK(array->ValidateFull());
}

TEST_P(RandomArrayTest, GenerateBatch) {
  auto field = GetField();
  auto batch = GenerateBatch({field}, kExpectedLength, 0xDEADBEEF);
  AssertSchemaEqual(schema({field}), batch->schema());
  auto array = batch->column(0);
  ASSERT_EQ(kExpectedLength, array->length());
  ASSERT_OK(array->ValidateFull());
}

TEST_P(RandomArrayTest, GenerateZeroLengthArray) {
  auto field = GetField();
  if (field->type()->id() == Type::type::DENSE_UNION) {
    GTEST_SKIP() << "Cannot generate zero-length dense union arrays";
  }
  auto array = GenerateArray(*field, 0, 0xDEADBEEF);
  AssertTypeEqual(field->type(), array->type());
  ASSERT_EQ(0, array->length());
  ASSERT_OK(array->ValidateFull());
}

TEST_P(RandomArrayTest, GenerateArrayWithZeroNullProbability) {
  auto field =
      GetField()->WithMetadata(key_value_metadata({{"null_probability", "0.0"}}));
  if (field->type()->id() == Type::type::NA) {
    GTEST_SKIP() << "Cannot generate non-null null arrays";
  }
  auto batch = GenerateBatch({field}, kExpectedLength, 0xDEADBEEF);
  AssertSchemaEqual(schema({field}), batch->schema());
  auto array = batch->column(0);
  ASSERT_OK(array->ValidateFull());
  ASSERT_EQ(0, array->null_count());
}

TEST_P(RandomArrayTest, GenerateNonNullableArray) {
  auto field = GetField()->WithNullable(false);
  if (field->type()->id() == Type::type::NA) {
    GTEST_SKIP() << "Cannot generate non-null null arrays";
  }
  auto batch = GenerateBatch({field}, kExpectedLength, 0xDEADBEEF);
  AssertSchemaEqual(schema({field}), batch->schema());
  auto array = batch->column(0);
  ASSERT_OK(array->ValidateFull());
  ASSERT_EQ(0, array->null_count());
}

auto values = ::testing::Values(
    field("null", null()), field("bool", boolean()), field("uint8", uint8()),
    field("int8", int8()), field("uint16", uint16()), field("int16", int16()),
    field("uint32", uint32()), field("int32", int32()), field("uint64", uint64()),
    field("int64", int64()), field("float16", float16()), field("float32", float32()),
    field("float64", float64()), field("string", utf8()), field("binary", binary()),
    field("fixed_size_binary", fixed_size_binary(8)),
    field("decimal128", decimal128(8, 3)), field("decimal256", decimal256(16, 4)),
    field("date32", date32()), field("date64", date64()),
    field("timestampns", timestamp(TimeUnit::NANO)),
    field("timestamps", timestamp(TimeUnit::SECOND, "America/Phoenix")),
    field("time32ms", time32(TimeUnit::MILLI)), field("time64ns", time64(TimeUnit::NANO)),
    field("time32s", time32(TimeUnit::SECOND)),
    field("time64us", time64(TimeUnit::MICRO)), field("month_interval", month_interval()),
    field("daytime_interval", day_time_interval()), field("listint8", list(int8())),
    field("listlistint8", list(list(int8()))),
    field("listint8emptynulls", list(int8()), true,
          key_value_metadata({{"force_empty_nulls", "true"}})),
    field("listint81024values", list(int8()), true,
          key_value_metadata({{"values", "1024"}})),
    field("structints", struct_({
                            field("int8", int8()),
                            field("int16", int16()),
                            field("int32", int32()),
                        })),
    field("structnested", struct_({
                              field("string", utf8()),
                              field("list", list(int64())),
                              field("timestamp", timestamp(TimeUnit::MILLI)),
                          })),
    field("sparseunion", sparse_union({
                             field("int8", int8()),
                             field("int16", int16()),
                             field("int32", int32()),
                         })),
    field("denseunion", dense_union({
                            field("int8", int8()),
                            field("int16", int16()),
                            field("int32", int32()),
                        })),
    field("dictionary", dictionary(int8(), utf8())), field("map", map(int8(), utf8())),
    field("fixedsizelist", fixed_size_list(int8(), 4)),
    field("durationns", duration(TimeUnit::NANO)), field("largestring", large_utf8()),
    field("largebinary", large_binary()),
    field("largelistlistint8", large_list(list(int8()))));

INSTANTIATE_TEST_SUITE_P(
    TestRandomArrayGeneration, RandomArrayTest, values,
    [](const ::testing::TestParamInfo<RandomArrayTest::ParamType>& info) {
      return std::to_string(info.index) + info.param->name();
    });

using NumericTypes =
    ::testing::Types<UInt8Type, Int8Type, UInt16Type, Int16Type, UInt32Type, Int32Type,
                     HalfFloatType, FloatType, DoubleType>;
TYPED_TEST_SUITE(RandomNumericArrayTest, NumericTypes);

TYPED_TEST(RandomNumericArrayTest, GenerateMinMax) {
  auto field = this->GetField()->WithMetadata(
      key_value_metadata({{"min", "0"}, {"max", "127"}, {"nan_probability", "0.0"}}));
  auto batch = GenerateBatch({field}, kExpectedLength, 0xDEADBEEF);
  ASSERT_OK(batch->ValidateFull());
  AssertSchemaEqual(schema({field}), batch->schema());
  auto array = this->Downcast(batch->column(0));
  for (auto slot : *array) {
    if (!slot.has_value()) continue;
    ASSERT_GE(slot, typename TypeParam::c_type(0));
    ASSERT_LE(slot, typename TypeParam::c_type(127));
  }
}

// Test all the supported options
TEST(TypeSpecificTests, BoolTrueProbability) {
  auto field =
      arrow::field("bool", boolean(), key_value_metadata({{"true_probability", "1.0"}}));
  auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), base_array->type());
  auto array = internal::checked_pointer_cast<BooleanArray>(base_array);
  ASSERT_OK(array->ValidateFull());
  for (const auto& value : *array) {
    ASSERT_TRUE(!value.has_value() || *value);
  }
}

TEST(TypeSpecificTests, DictionaryValues) {
  auto field = arrow::field("dictionary", dictionary(int8(), utf8()),
                            key_value_metadata({{"values", "16"}}));
  auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), base_array->type());
  auto array = internal::checked_pointer_cast<DictionaryArray>(base_array);
  ASSERT_OK(array->ValidateFull());
  ASSERT_EQ(16, array->dictionary()->length());
}

TEST(TypeSpecificTests, Float32Nan) {
  auto field = arrow::field("float32", float32(),
                            key_value_metadata({{"nan_probability", "1.0"}}));
  auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), base_array->type());
  auto array = internal::checked_pointer_cast<NumericArray<FloatType>>(base_array);
  ASSERT_OK(array->ValidateFull());
  for (const auto& value : *array) {
    ASSERT_TRUE(!value.has_value() || std::isnan(*value));
  }
}

TEST(TypeSpecificTests, Float64Nan) {
  auto field = arrow::field("float64", float64(),
                            key_value_metadata({{"nan_probability", "1.0"}}));
  auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), base_array->type());
  auto array = internal::checked_pointer_cast<NumericArray<DoubleType>>(base_array);
  ASSERT_OK(array->ValidateFull());
  for (const auto& value : *array) {
    ASSERT_TRUE(!value.has_value() || std::isnan(*value));
  }
}

TEST(TypeSpecificTests, ListLengths) {
  {
    auto field =
        arrow::field("list", list(int8()),
                     key_value_metadata({{"min_length", "1"}, {"max_length", "1"}}));
    auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
    AssertTypeEqual(field->type(), base_array->type());
    auto array = internal::checked_pointer_cast<ListArray>(base_array);
    ASSERT_OK(array->ValidateFull());
    ASSERT_EQ(array->length(), kExpectedLength);
    for (int i = 0; i < kExpectedLength; i++) {
      if (!array->IsNull(i)) {
        ASSERT_EQ(1, array->value_length(i));
      }
    }
  }
  {
    auto field =
        arrow::field("list", large_list(int8()),
                     key_value_metadata({{"min_length", "10"}, {"max_length", "10"}}));
    auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
    AssertTypeEqual(field->type(), base_array->type());
    auto array = internal::checked_pointer_cast<LargeListArray>(base_array);
    ASSERT_EQ(array->length(), kExpectedLength);
    ASSERT_OK(array->ValidateFull());
    for (int i = 0; i < kExpectedLength; i++) {
      if (!array->IsNull(i)) {
        ASSERT_EQ(10, array->value_length(i));
      }
    }
  }
}

TEST(TypeSpecificTests, MapValues) {
  auto field =
      arrow::field("map", map(int8(), int8()), key_value_metadata({{"values", "4"}}));
  auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), base_array->type());
  auto array = internal::checked_pointer_cast<MapArray>(base_array);
  ASSERT_OK(array->ValidateFull());
  ASSERT_EQ(4, array->keys()->length());
  ASSERT_EQ(4, array->items()->length());
}

TEST(TypeSpecificTests, RepeatedStrings) {
  auto field = arrow::field("string", utf8(), key_value_metadata({{"unique", "1"}}));
  auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
  AssertTypeEqual(field->type(), base_array->type());
  auto array = internal::checked_pointer_cast<StringArray>(base_array);
  ASSERT_OK(array->ValidateFull());
  util::string_view singular_value = array->GetView(0);
  for (auto slot : *array) {
    if (!slot.has_value()) continue;
    ASSERT_EQ(slot, singular_value);
  }
  // N.B. LargeString does not support unique
}

TEST(TypeSpecificTests, StringLengths) {
  {
    auto field = arrow::field(
        "list", utf8(), key_value_metadata({{"min_length", "1"}, {"max_length", "1"}}));
    auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
    AssertTypeEqual(field->type(), base_array->type());
    auto array = internal::checked_pointer_cast<StringArray>(base_array);
    ASSERT_OK(array->ValidateFull());
    for (int i = 0; i < kExpectedLength; i++) {
      if (!array->IsNull(i)) {
        ASSERT_EQ(1, array->value_length(i));
      }
    }
  }
  {
    auto field = arrow::field(
        "list", binary(), key_value_metadata({{"min_length", "1"}, {"max_length", "1"}}));
    auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
    AssertTypeEqual(field->type(), base_array->type());
    auto array = internal::checked_pointer_cast<BinaryArray>(base_array);
    ASSERT_OK(array->ValidateFull());
    for (int i = 0; i < kExpectedLength; i++) {
      if (!array->IsNull(i)) {
        ASSERT_EQ(1, array->value_length(i));
      }
    }
  }
  {
    auto field =
        arrow::field("list", large_utf8(),
                     key_value_metadata({{"min_length", "10"}, {"max_length", "10"}}));
    auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
    AssertTypeEqual(field->type(), base_array->type());
    auto array = internal::checked_pointer_cast<LargeStringArray>(base_array);
    ASSERT_OK(array->ValidateFull());
    for (int i = 0; i < kExpectedLength; i++) {
      if (!array->IsNull(i)) {
        ASSERT_EQ(10, array->value_length(i));
      }
    }
  }
  {
    auto field =
        arrow::field("list", large_binary(),
                     key_value_metadata({{"min_length", "10"}, {"max_length", "10"}}));
    auto base_array = GenerateArray(*field, kExpectedLength, 0xDEADBEEF);
    AssertTypeEqual(field->type(), base_array->type());
    auto array = internal::checked_pointer_cast<LargeBinaryArray>(base_array);
    ASSERT_OK(array->ValidateFull());
    for (int i = 0; i < kExpectedLength; i++) {
      if (!array->IsNull(i)) {
        ASSERT_EQ(10, array->value_length(i));
      }
    }
  }
}

TEST(RandomList, Basics) {
  random::RandomArrayGenerator rng(42);
  for (const double null_probability : {0.0, 0.1, 0.98}) {
    SCOPED_TRACE("null_probability = " + std::to_string(null_probability));
    auto values = rng.Int16(1234, 0, 10000, null_probability);
    auto array = rng.List(*values, 45, null_probability);
    ASSERT_OK(array->ValidateFull());
    ASSERT_EQ(array->length(), 45);
    const auto& list_array = checked_cast<const ListArray&>(*array);
    ASSERT_EQ(list_array.values()->length(), 1234);
    int64_t null_count = 0;
    for (int64_t i = 0; i < array->length(); ++i) {
      null_count += array->IsNull(i);
    }
    ASSERT_EQ(null_count, array->data()->null_count);
  }
}

}  // namespace random
}  // namespace arrow
