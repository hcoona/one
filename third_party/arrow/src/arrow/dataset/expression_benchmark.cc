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

#include "third_party/benchmark/include/benchmark/benchmark.h"

#include "third_party/arrow/src/arrow/compute/cast.h"
#include "third_party/arrow/src/arrow/dataset/expression.h"
#include "third_party/arrow/src/arrow/dataset/partition.h"
#include "third_party/arrow/src/arrow/testing/gtest_util.h"
#include "third_party/arrow/src/arrow/type.h"

namespace arrow {
namespace dataset {

static Expression GetPartitionExpression(const std::string& path, bool infer_dictionary) {
  auto options = HivePartitioningFactoryOptions();
  options.infer_dictionary = infer_dictionary;
  auto factory = HivePartitioning::MakeFactory(options);
  ASSIGN_OR_ABORT(auto schema, factory->Inspect({path}));
  ASSIGN_OR_ABORT(auto partitioning, factory->Finish(schema));
  ASSIGN_OR_ABORT(auto expr, partitioning->Parse(path));
  return expr;
}

// A benchmark of SimplifyWithGuarantee using expressions arising from partitioning.
static void SimplifyFilterWithGuarantee(benchmark::State& state, Expression filter,
                                        Expression guarantee) {
  auto dataset_schema = schema({field("a", int64()), field("b", int64())});
  ASSIGN_OR_ABORT(filter, filter.Bind(*dataset_schema));

  for (auto _ : state) {
    ABORT_NOT_OK(SimplifyWithGuarantee(filter, guarantee));
  }
}

auto to_int64 = compute::CastOptions::Safe(int64());
// A fully simplified filter.
auto filter_simple_negative = and_(equal(field_ref("a"), literal(int64_t(99))),
                                   equal(field_ref("b"), literal(int64_t(98))));
auto filter_simple_positive = and_(equal(field_ref("a"), literal(int64_t(99))),
                                   equal(field_ref("b"), literal(int64_t(99))));
// A filter with casts inserted due to converting between the
// assumed-by-default type and the inferred partition schema.
auto filter_cast_negative =
    and_(equal(call("cast", {field_ref("a")}, to_int64), literal(99)),
         equal(call("cast", {field_ref("b")}, to_int64), literal(98)));
auto filter_cast_positive =
    and_(equal(call("cast", {field_ref("a")}, to_int64), literal(99)),
         equal(call("cast", {field_ref("b")}, to_int64), literal(99)));
// A fully simplified partition expression.
auto guarantee = GetPartitionExpression("a=99/b=99", /*infer_dictionary=*/false);
// A partition expression that uses dictionaries, which are inferred by default.
auto guarantee_dictionary =
    GetPartitionExpression("a=99/b=99", /*infer_dictionary=*/true);
// Negative queries (partition expressions that fail the filter)
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee, negative_filter_simple_guarantee_simple,
                  filter_simple_negative, guarantee);
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee, negative_filter_cast_guarantee_simple,
                  filter_cast_negative, guarantee);
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee,
                  negative_filter_simple_guarantee_dictionary, filter_simple_negative,
                  guarantee_dictionary);
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee, negative_filter_cast_guarantee_dictionary,
                  filter_cast_negative, guarantee_dictionary);
// Positive queries (partition expressions that pass the filter)
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee, positive_filter_simple_guarantee_simple,
                  filter_simple_positive, guarantee);
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee, positive_filter_cast_guarantee_simple,
                  filter_cast_positive, guarantee);
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee,
                  positive_filter_simple_guarantee_dictionary, filter_simple_positive,
                  guarantee_dictionary);
BENCHMARK_CAPTURE(SimplifyFilterWithGuarantee, positive_filter_cast_guarantee_dictionary,
                  filter_cast_positive, guarantee_dictionary);

}  // namespace dataset
}  // namespace arrow
