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

#include <algorithm>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "third_party/googletest/googletest/include/gtest/gtest.h"

#include "third_party/arrow/src/arrow/array.h"
#include "third_party/arrow/src/arrow/chunked_array.h"
#include "third_party/arrow/src/arrow/compute/api_aggregate.h"
#include "third_party/arrow/src/arrow/compute/api_scalar.h"
#include "third_party/arrow/src/arrow/compute/api_vector.h"
#include "third_party/arrow/src/arrow/compute/cast.h"
#include "third_party/arrow/src/arrow/compute/kernels/aggregate_internal.h"
#include "third_party/arrow/src/arrow/compute/kernels/codegen_internal.h"
#include "third_party/arrow/src/arrow/compute/kernels/test_util.h"
#include "third_party/arrow/src/arrow/compute/registry.h"
#include "third_party/arrow/src/arrow/testing/generator.h"
#include "third_party/arrow/src/arrow/testing/gtest_common.h"
#include "third_party/arrow/src/arrow/testing/gtest_util.h"
#include "third_party/arrow/src/arrow/testing/random.h"
#include "third_party/arrow/src/arrow/type.h"
#include "third_party/arrow/src/arrow/type_traits.h"
#include "third_party/arrow/src/arrow/util/bitmap_reader.h"
#include "third_party/arrow/src/arrow/util/checked_cast.h"
#include "third_party/arrow/src/arrow/util/int_util_internal.h"
#include "third_party/arrow/src/arrow/util/key_value_metadata.h"
#include "third_party/arrow/src/arrow/util/logging.h"

using testing::HasSubstr;

namespace arrow {

using internal::BitmapReader;
using internal::checked_cast;
using internal::checked_pointer_cast;

namespace compute {
namespace {

Result<Datum> NaiveGroupBy(std::vector<Datum> arguments, std::vector<Datum> keys,
                           const std::vector<internal::Aggregate>& aggregates) {
  ARROW_ASSIGN_OR_RAISE(auto key_batch, ExecBatch::Make(std::move(keys)));

  ARROW_ASSIGN_OR_RAISE(auto grouper,
                        internal::Grouper::Make(key_batch.GetDescriptors()));

  ARROW_ASSIGN_OR_RAISE(Datum id_batch, grouper->Consume(key_batch));

  ARROW_ASSIGN_OR_RAISE(
      auto groupings, internal::Grouper::MakeGroupings(*id_batch.array_as<UInt32Array>(),
                                                       grouper->num_groups()));

  ArrayVector out_columns;
  std::vector<std::string> out_names;

  for (size_t i = 0; i < arguments.size(); ++i) {
    out_names.push_back(aggregates[i].function);

    // trim "hash_" prefix
    auto scalar_agg_function = aggregates[i].function.substr(5);

    ARROW_ASSIGN_OR_RAISE(
        auto grouped_argument,
        internal::Grouper::ApplyGroupings(*groupings, *arguments[i].make_array()));

    ScalarVector aggregated_scalars;

    for (int64_t i_group = 0; i_group < grouper->num_groups(); ++i_group) {
      auto slice = grouped_argument->value_slice(i_group);
      if (slice->length() == 0) continue;
      ARROW_ASSIGN_OR_RAISE(
          Datum d, CallFunction(scalar_agg_function, {slice}, aggregates[i].options));
      aggregated_scalars.push_back(d.scalar());
    }

    ARROW_ASSIGN_OR_RAISE(Datum aggregated_column,
                          ScalarVectorToArray(aggregated_scalars));
    out_columns.push_back(aggregated_column.make_array());
  }

  int i = 0;
  ARROW_ASSIGN_OR_RAISE(auto uniques, grouper->GetUniques());
  for (const Datum& key : uniques.values) {
    out_columns.push_back(key.make_array());
    out_names.push_back("key_" + std::to_string(i++));
  }

  return StructArray::Make(std::move(out_columns), std::move(out_names));
}

void ValidateGroupBy(const std::vector<internal::Aggregate>& aggregates,
                     std::vector<Datum> arguments, std::vector<Datum> keys) {
  ASSERT_OK_AND_ASSIGN(Datum expected, NaiveGroupBy(arguments, keys, aggregates));

  ASSERT_OK_AND_ASSIGN(Datum actual, GroupBy(arguments, keys, aggregates));

  ASSERT_OK(expected.make_array()->ValidateFull());
  ASSERT_OK(actual.make_array()->ValidateFull());

  AssertDatumsEqual(expected, actual, /*verbose=*/true);
}

}  // namespace

TEST(Grouper, SupportedKeys) {
  ASSERT_OK(internal::Grouper::Make({boolean()}));

  ASSERT_OK(internal::Grouper::Make({int8(), uint16(), int32(), uint64()}));

  ASSERT_OK(internal::Grouper::Make({dictionary(int64(), utf8())}));

  ASSERT_OK(internal::Grouper::Make({float16(), float32(), float64()}));

  ASSERT_OK(internal::Grouper::Make({utf8(), binary(), large_utf8(), large_binary()}));

  ASSERT_OK(internal::Grouper::Make({fixed_size_binary(16), fixed_size_binary(32)}));

  ASSERT_OK(internal::Grouper::Make({decimal128(32, 10), decimal256(76, 20)}));

  ASSERT_OK(internal::Grouper::Make({date32(), date64()}));

  for (auto unit : {
           TimeUnit::SECOND,
           TimeUnit::MILLI,
           TimeUnit::MICRO,
           TimeUnit::NANO,
       }) {
    ASSERT_OK(internal::Grouper::Make({timestamp(unit), duration(unit)}));
  }

  ASSERT_OK(internal::Grouper::Make({day_time_interval(), month_interval()}));

  ASSERT_RAISES(NotImplemented, internal::Grouper::Make({struct_({field("", int64())})}));

  ASSERT_RAISES(NotImplemented, internal::Grouper::Make({struct_({})}));

  ASSERT_RAISES(NotImplemented, internal::Grouper::Make({list(int32())}));

  ASSERT_RAISES(NotImplemented, internal::Grouper::Make({fixed_size_list(int32(), 5)}));

  ASSERT_RAISES(NotImplemented,
                internal::Grouper::Make({dense_union({field("", int32())})}));
}

struct TestGrouper {
  explicit TestGrouper(std::vector<ValueDescr> descrs) : descrs_(std::move(descrs)) {
    grouper_ = internal::Grouper::Make(descrs_).ValueOrDie();

    FieldVector fields;
    for (const auto& descr : descrs_) {
      fields.push_back(field("", descr.type));
    }
    key_schema_ = schema(std::move(fields));
  }

  void ExpectConsume(const std::string& key_json, const std::string& expected) {
    ExpectConsume(ExecBatch(*RecordBatchFromJSON(key_schema_, key_json)),
                  ArrayFromJSON(uint32(), expected));
  }

  void ExpectConsume(const std::vector<Datum>& key_batch, Datum expected) {
    ExpectConsume(*ExecBatch::Make(key_batch), expected);
  }

  void ExpectConsume(const ExecBatch& key_batch, Datum expected) {
    Datum ids;
    ConsumeAndValidate(key_batch, &ids);
    AssertDatumsEqual(expected, ids, /*verbose=*/true);
  }

  void ConsumeAndValidate(const ExecBatch& key_batch, Datum* ids = nullptr) {
    ASSERT_OK_AND_ASSIGN(Datum id_batch, grouper_->Consume(key_batch));

    ValidateConsume(key_batch, id_batch);

    if (ids) {
      *ids = std::move(id_batch);
    }
  }

  void ValidateConsume(const ExecBatch& key_batch, const Datum& id_batch) {
    if (uniques_.length == -1) {
      ASSERT_OK_AND_ASSIGN(uniques_, grouper_->GetUniques());
    } else if (static_cast<int64_t>(grouper_->num_groups()) > uniques_.length) {
      ASSERT_OK_AND_ASSIGN(ExecBatch new_uniques, grouper_->GetUniques());

      // check that uniques_ are prefixes of new_uniques
      for (int i = 0; i < uniques_.num_values(); ++i) {
        auto new_unique = new_uniques[i].make_array();
        ASSERT_OK(new_unique->ValidateFull());

        AssertDatumsEqual(uniques_[i], new_unique->Slice(0, uniques_.length),
                          /*verbose=*/true);
      }

      uniques_ = std::move(new_uniques);
    }

    // check that the ids encode an equivalent key sequence
    auto ids = id_batch.make_array();
    ASSERT_OK(ids->ValidateFull());

    for (int i = 0; i < key_batch.num_values(); ++i) {
      SCOPED_TRACE(std::to_string(i) + "th key array");
      auto original = key_batch[i].make_array();
      ASSERT_OK_AND_ASSIGN(auto encoded, Take(*uniques_[i].make_array(), *ids));
      AssertArraysEqual(*original, *encoded, /*verbose=*/true,
                        EqualOptions().nans_equal(true));
    }
  }

  std::vector<ValueDescr> descrs_;
  std::shared_ptr<Schema> key_schema_;
  std::unique_ptr<internal::Grouper> grouper_;
  ExecBatch uniques_ = ExecBatch({}, -1);
};

TEST(Grouper, BooleanKey) {
  TestGrouper g({boolean()});

  g.ExpectConsume("[[true], [true]]", "[0, 0]");

  g.ExpectConsume("[[true], [true]]", "[0, 0]");

  g.ExpectConsume("[[false], [null]]", "[1, 2]");

  g.ExpectConsume("[[true], [false], [true], [false], [null], [false], [null]]",
                  "[0, 1, 0, 1, 2, 1, 2]");
}

TEST(Grouper, NumericKey) {
  for (auto ty : {
           uint8(),
           int8(),
           uint16(),
           int16(),
           uint32(),
           int32(),
           uint64(),
           int64(),
           float16(),
           float32(),
           float64(),
       }) {
    SCOPED_TRACE("key type: " + ty->ToString());

    TestGrouper g({ty});

    g.ExpectConsume("[[3], [3]]", "[0, 0]");

    g.ExpectConsume("[[3], [3]]", "[0, 0]");

    g.ExpectConsume("[[27], [81]]", "[1, 2]");

    g.ExpectConsume("[[3], [27], [3], [27], [null], [81], [27], [81]]",
                    "[0, 1, 0, 1, 3, 2, 1, 2]");
  }
}

TEST(Grouper, FloatingPointKey) {
  TestGrouper g({float32()});

  // -0.0 hashes differently from 0.0
  g.ExpectConsume("[[0.0], [-0.0]]", "[0, 1]");

  g.ExpectConsume("[[Inf], [-Inf]]", "[2, 3]");

  // assert(!(NaN == NaN)) does not cause spurious new groups
  g.ExpectConsume("[[NaN], [NaN]]", "[4, 4]");

  // TODO(bkietz) test denormal numbers, more NaNs
}

TEST(Grouper, StringKey) {
  for (auto ty : {utf8(), large_utf8(), fixed_size_binary(2)}) {
    SCOPED_TRACE("key type: " + ty->ToString());

    TestGrouper g({ty});

    g.ExpectConsume(R"([["eh"], ["eh"]])", "[0, 0]");

    g.ExpectConsume(R"([["eh"], ["eh"]])", "[0, 0]");

    g.ExpectConsume(R"([["be"], [null]])", "[1, 2]");
  }
}

TEST(Grouper, DictKey) {
  TestGrouper g({dictionary(int32(), utf8())});

  // For dictionary keys, all batches must share a single dictionary.
  // Eventually, differing dictionaries will be unified and indices transposed
  // during encoding to relieve this restriction.
  const auto dict = ArrayFromJSON(utf8(), R"(["ex", "why", "zee", null])");

  auto WithIndices = [&](const std::string& indices) {
    return Datum(*DictionaryArray::FromArrays(ArrayFromJSON(int32(), indices), dict));
  };

  // NB: null index is not considered equivalent to index=3 (which encodes null in dict)
  g.ExpectConsume({WithIndices("           [3, 1, null, 0, 2]")},
                  ArrayFromJSON(uint32(), "[0, 1, 2, 3, 4]"));

  g = TestGrouper({dictionary(int32(), utf8())});

  g.ExpectConsume({WithIndices("           [0, 1, 2, 3, null]")},
                  ArrayFromJSON(uint32(), "[0, 1, 2, 3, 4]"));

  g.ExpectConsume({WithIndices("           [3, 1, null, 0, 2]")},
                  ArrayFromJSON(uint32(), "[3, 1, 4,    0, 2]"));

  EXPECT_RAISES_WITH_MESSAGE_THAT(
      NotImplemented, HasSubstr("Unifying differing dictionaries"),
      g.grouper_->Consume(*ExecBatch::Make({*DictionaryArray::FromArrays(
          ArrayFromJSON(int32(), "[0, 1]"),
          ArrayFromJSON(utf8(), R"(["different", "dictionary"])"))})));
}

TEST(Grouper, StringInt64Key) {
  TestGrouper g({utf8(), int64()});

  g.ExpectConsume(R"([["eh", 0], ["eh", 0]])", "[0, 0]");

  g.ExpectConsume(R"([["eh", 0], ["eh", null]])", "[0, 1]");

  g.ExpectConsume(R"([["eh", 1], ["bee", 1]])", "[2, 3]");

  g.ExpectConsume(R"([["eh", null], ["bee", 1]])", "[1, 3]");

  g = TestGrouper({utf8(), int64()});

  g.ExpectConsume(R"([
    ["ex",  0],
    ["ex",  0],
    ["why", 0],
    ["ex",  1],
    ["why", 0],
    ["ex",  1],
    ["ex",  0],
    ["why", 1]
  ])",
                  "[0, 0, 1, 2, 1, 2, 0, 3]");

  g.ExpectConsume(R"([
    ["ex",  0],
    [null,  0],
    [null,  0],
    ["ex",  1],
    [null,  null],
    ["ex",  1],
    ["ex",  0],
    ["why", null]
  ])",
                  "[0, 4, 4, 2, 5, 2, 0, 6]");
}

TEST(Grouper, DoubleStringInt64Key) {
  TestGrouper g({float64(), utf8(), int64()});

  g.ExpectConsume(R"([[1.5, "eh", 0], [1.5, "eh", 0]])", "[0, 0]");

  g.ExpectConsume(R"([[1.5, "eh", 0], [1.5, "eh", 0]])", "[0, 0]");

  g.ExpectConsume(R"([[1.0, "eh", 0], [1.0, "be", null]])", "[1, 2]");

  // note: -0 and +0 hash differently
  g.ExpectConsume(R"([[-0.0, "be", 7], [0.0, "be", 7]])", "[3, 4]");
}

TEST(Grouper, RandomInt64Keys) {
  TestGrouper g({int64()});
  for (int i = 0; i < 4; ++i) {
    SCOPED_TRACE(std::to_string(i) + "th key batch");

    ExecBatch key_batch{
        *random::GenerateBatch(g.key_schema_->fields(), 1 << 12, 0xDEADBEEF)};
    g.ConsumeAndValidate(key_batch);
  }
}

TEST(Grouper, RandomStringInt64Keys) {
  TestGrouper g({utf8(), int64()});
  for (int i = 0; i < 4; ++i) {
    SCOPED_TRACE(std::to_string(i) + "th key batch");

    ExecBatch key_batch{
        *random::GenerateBatch(g.key_schema_->fields(), 1 << 12, 0xDEADBEEF)};
    g.ConsumeAndValidate(key_batch);
  }
}

TEST(Grouper, RandomStringInt64DoubleInt32Keys) {
  TestGrouper g({utf8(), int64(), float64(), int32()});
  for (int i = 0; i < 4; ++i) {
    SCOPED_TRACE(std::to_string(i) + "th key batch");

    ExecBatch key_batch{
        *random::GenerateBatch(g.key_schema_->fields(), 1 << 12, 0xDEADBEEF)};
    g.ConsumeAndValidate(key_batch);
  }
}

TEST(Grouper, MakeGroupings) {
  auto ExpectGroupings = [](std::string ids_json, std::string expected_json) {
    auto ids = checked_pointer_cast<UInt32Array>(ArrayFromJSON(uint32(), ids_json));
    auto expected = ArrayFromJSON(list(int32()), expected_json);

    auto num_groups = static_cast<uint32_t>(expected->length());
    ASSERT_OK_AND_ASSIGN(auto actual, internal::Grouper::MakeGroupings(*ids, num_groups));
    AssertArraysEqual(*expected, *actual, /*verbose=*/true);

    // validate ApplyGroupings
    ASSERT_OK_AND_ASSIGN(auto grouped_ids,
                         internal::Grouper::ApplyGroupings(*actual, *ids));

    for (uint32_t group = 0; group < num_groups; ++group) {
      auto ids_slice = checked_pointer_cast<UInt32Array>(grouped_ids->value_slice(group));
      for (auto slot : *ids_slice) {
        EXPECT_EQ(slot, group);
      }
    }
  };

  ExpectGroupings("[]", "[[]]");

  ExpectGroupings("[0, 0, 0]", "[[0, 1, 2]]");

  ExpectGroupings("[0, 0, 0, 1, 1, 2]", "[[0, 1, 2], [3, 4], [5], []]");

  ExpectGroupings("[2, 1, 2, 1, 1, 2]", "[[], [1, 3, 4], [0, 2, 5], [], []]");

  ExpectGroupings("[2, 2, 5, 5, 2, 3]", "[[], [], [0, 1, 4], [5], [], [2, 3], [], []]");

  auto ids = checked_pointer_cast<UInt32Array>(ArrayFromJSON(uint32(), "[0, null, 1]"));
  EXPECT_RAISES_WITH_MESSAGE_THAT(Invalid, HasSubstr("MakeGroupings with null ids"),
                                  internal::Grouper::MakeGroupings(*ids, 5));
}

TEST(GroupBy, Errors) {
  auto batch = RecordBatchFromJSON(
      schema({field("argument", float64()), field("group_id", uint32())}), R"([
    [1.0,   1],
    [null,  1],
    [0.0,   2],
    [null,  3],
    [4.0,   0],
    [3.25,  1],
    [0.125, 2],
    [-0.25, 2],
    [0.75,  0],
    [null,  3]
  ])");

  EXPECT_RAISES_WITH_MESSAGE_THAT(
      NotImplemented, HasSubstr("Direct execution of HASH_AGGREGATE functions"),
      CallFunction("hash_sum", {batch->GetColumnByName("argument"),
                                batch->GetColumnByName("group_id"), Datum(uint32_t(4))}));
}

TEST(GroupBy, SumOnly) {
  auto batch = RecordBatchFromJSON(
      schema({field("argument", float64()), field("key", int64())}), R"([
    [1.0,   1],
    [null,  1],
    [0.0,   2],
    [null,  3],
    [4.0,   null],
    [3.25,  1],
    [0.125, 2],
    [-0.25, 2],
    [0.75,  null],
    [null,  3]
  ])");

  ASSERT_OK_AND_ASSIGN(Datum aggregated_and_grouped,
                       internal::GroupBy({batch->GetColumnByName("argument")},
                                         {batch->GetColumnByName("key")},
                                         {
                                             {"hash_sum", nullptr},
                                         }));

  AssertDatumsEqual(ArrayFromJSON(struct_({
                                      field("hash_sum", float64()),
                                      field("key_0", int64()),
                                  }),
                                  R"([
    [4.25,   1],
    [-0.125, 2],
    [null,   3],
    [4.75,   null]
  ])"),
                    aggregated_and_grouped,
                    /*verbose=*/true);
}

TEST(GroupBy, MinMaxOnly) {
  auto batch = RecordBatchFromJSON(
      schema({field("argument", float64()), field("key", int64())}), R"([
    [1.0,   1],
    [null,  1],
    [0.0,   2],
    [null,  3],
    [4.0,   null],
    [3.25,  1],
    [0.125, 2],
    [-0.25, 2],
    [0.75,  null],
    [null,  3]
  ])");

  ASSERT_OK_AND_ASSIGN(Datum aggregated_and_grouped,
                       internal::GroupBy({batch->GetColumnByName("argument")},
                                         {batch->GetColumnByName("key")},
                                         {
                                             {"hash_min_max", nullptr},
                                         }));

  AssertDatumsEqual(ArrayFromJSON(struct_({
                                      field("hash_min_max", struct_({
                                                                field("min", float64()),
                                                                field("max", float64()),
                                                            })),
                                      field("key_0", int64()),
                                  }),
                                  R"([
    [{"min": 1.0,   "max": 3.25},  1],
    [{"min": -0.25, "max": 0.125}, 2],
    [{"min": null,  "max": null},  3],
    [{"min": 0.75,  "max": 4.0},   null]
  ])"),
                    aggregated_and_grouped,
                    /*verbose=*/true);
}

TEST(GroupBy, CountAndSum) {
  auto batch = RecordBatchFromJSON(
      schema({field("argument", float64()), field("key", int64())}), R"([
    [1.0,   1],
    [null,  1],
    [0.0,   2],
    [null,  3],
    [4.0,   null],
    [3.25,  1],
    [0.125, 2],
    [-0.25, 2],
    [0.75,  null],
    [null,  3]
  ])");

  CountOptions count_options;
  ASSERT_OK_AND_ASSIGN(
      Datum aggregated_and_grouped,
      internal::GroupBy(
          {
              // NB: passing an argument twice or also using it as a key is legal
              batch->GetColumnByName("argument"),
              batch->GetColumnByName("argument"),
              batch->GetColumnByName("key"),
          },
          {
              batch->GetColumnByName("key"),
          },
          {
              {"hash_count", &count_options},
              {"hash_sum", nullptr},
              {"hash_sum", nullptr},
          }));

  AssertDatumsEqual(
      ArrayFromJSON(struct_({
                        field("hash_count", int64()),
                        // NB: summing a float32 array results in float64 sums
                        field("hash_sum", float64()),
                        field("hash_sum", int64()),
                        field("key_0", int64()),
                    }),
                    R"([
    [2, 4.25,   3,    1],
    [3, -0.125, 6,    2],
    [0, null,   6,    3],
    [2, 4.75,   null, null]
  ])"),
      aggregated_and_grouped,
      /*verbose=*/true);
}

TEST(GroupBy, SumOnlyStringAndDictKeys) {
  for (auto key_type : {utf8(), dictionary(int32(), utf8())}) {
    SCOPED_TRACE("key type: " + key_type->ToString());

    auto batch = RecordBatchFromJSON(
        schema({field("argument", float64()), field("key", key_type)}), R"([
      [1.0,   "alfa"],
      [null,  "alfa"],
      [0.0,   "beta"],
      [null,  "gama"],
      [4.0,    null ],
      [3.25,  "alfa"],
      [0.125, "beta"],
      [-0.25, "beta"],
      [0.75,   null ],
      [null,  "gama"]
    ])");

    ASSERT_OK_AND_ASSIGN(Datum aggregated_and_grouped,
                         internal::GroupBy({batch->GetColumnByName("argument")},
                                           {batch->GetColumnByName("key")},
                                           {
                                               {"hash_sum", nullptr},
                                           }));

    AssertDatumsEqual(ArrayFromJSON(struct_({
                                        field("hash_sum", float64()),
                                        field("key_0", key_type),
                                    }),
                                    R"([
    [4.25,   "alfa"],
    [-0.125, "beta"],
    [null,   "gama"],
    [4.75,    null ]
  ])"),
                      aggregated_and_grouped,
                      /*verbose=*/true);
  }
}

TEST(GroupBy, ConcreteCaseWithValidateGroupBy) {
  auto batch = RecordBatchFromJSON(
      schema({field("argument", float64()), field("key", utf8())}), R"([
    [1.0,   "alfa"],
    [null,  "alfa"],
    [0.0,   "beta"],
    [null,  "gama"],
    [4.0,    null ],
    [3.25,  "alfa"],
    [0.125, "beta"],
    [-0.25, "beta"],
    [0.75,   null ],
    [null,  "gama"]
  ])");

  CountOptions count_non_null{CountOptions::COUNT_NON_NULL},
      count_null{CountOptions::COUNT_NULL};

  MinMaxOptions emit_null{MinMaxOptions::EMIT_NULL};

  using internal::Aggregate;
  for (auto agg : {
           Aggregate{"hash_sum", nullptr},
           Aggregate{"hash_count", &count_non_null},
           Aggregate{"hash_count", &count_null},
           Aggregate{"hash_min_max", nullptr},
           Aggregate{"hash_min_max", &emit_null},
       }) {
    SCOPED_TRACE(agg.function);
    ValidateGroupBy({agg}, {batch->GetColumnByName("argument")},
                    {batch->GetColumnByName("key")});
  }
}

// Count nulls/non_nulls from record batch with no nulls
TEST(GroupBy, CountNull) {
  auto batch = RecordBatchFromJSON(
      schema({field("argument", float64()), field("key", utf8())}), R"([
    [1.0, "alfa"],
    [2.0, "beta"],
    [3.0, "gama"]
  ])");

  CountOptions count_non_null{CountOptions::COUNT_NON_NULL},
      count_null{CountOptions::COUNT_NULL};

  using internal::Aggregate;
  for (auto agg : {
           Aggregate{"hash_count", &count_non_null},
           Aggregate{"hash_count", &count_null},
       }) {
    SCOPED_TRACE(agg.function);
    ValidateGroupBy({agg}, {batch->GetColumnByName("argument")},
                    {batch->GetColumnByName("key")});
  }
}

TEST(GroupBy, RandomArraySum) {
  for (int64_t length : {1 << 10, 1 << 12, 1 << 15}) {
    for (auto null_probability : {0.0, 0.01, 0.5, 1.0}) {
      auto batch = random::GenerateBatch(
          {
              field("argument", float32(),
                    key_value_metadata(
                        {{"null_probability", std::to_string(null_probability)}})),
              field("key", int64(), key_value_metadata({{"min", "0"}, {"max", "100"}})),
          },
          length, 0xDEADBEEF);

      ValidateGroupBy(
          {
              {"hash_sum", nullptr},
          },
          {batch->GetColumnByName("argument")}, {batch->GetColumnByName("key")});
    }
  }
}

}  // namespace compute
}  // namespace arrow
