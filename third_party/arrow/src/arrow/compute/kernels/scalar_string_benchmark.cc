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

#include "third_party/arrow/src/arrow/compute/api_scalar.h"
#include "third_party/arrow/src/arrow/compute/kernels/test_util.h"
#include "third_party/arrow/src/arrow/testing/gtest_util.h"
#include "third_party/arrow/src/arrow/testing/random.h"
#include "third_party/arrow/src/arrow/util/benchmark_util.h"

namespace arrow {
namespace compute {

constexpr auto kSeed = 0x94378165;

static void UnaryStringBenchmark(benchmark::State& state, const std::string& func_name,
                                 const FunctionOptions* options = nullptr) {
  const int64_t array_length = 1 << 20;
  const int64_t value_min_size = 0;
  const int64_t value_max_size = 32;
  const double null_probability = 0.01;
  random::RandomArrayGenerator rng(kSeed);

  // NOTE: this produces only-Ascii data
  auto values =
      rng.String(array_length, value_min_size, value_max_size, null_probability);
  // Make sure lookup tables are initialized before measuring
  ABORT_NOT_OK(CallFunction(func_name, {values}, options));

  for (auto _ : state) {
    ABORT_NOT_OK(CallFunction(func_name, {values}, options));
  }
  state.SetItemsProcessed(state.iterations() * array_length);
  state.SetBytesProcessed(state.iterations() * values->data()->buffers[2]->size());
}

static void AsciiLower(benchmark::State& state) {
  UnaryStringBenchmark(state, "ascii_lower");
}

static void AsciiUpper(benchmark::State& state) {
  UnaryStringBenchmark(state, "ascii_upper");
}

static void IsAlphaNumericAscii(benchmark::State& state) {
  UnaryStringBenchmark(state, "ascii_is_alnum");
}

static void MatchSubstring(benchmark::State& state) {
  MatchSubstringOptions options("abac");
  UnaryStringBenchmark(state, "match_substring", &options);
}

static void SplitPattern(benchmark::State& state) {
  SplitPatternOptions options("a");
  UnaryStringBenchmark(state, "split_pattern", &options);
}

static void TrimSingleAscii(benchmark::State& state) {
  TrimOptions options("a");
  UnaryStringBenchmark(state, "ascii_trim", &options);
}

static void TrimManyAscii(benchmark::State& state) {
  TrimOptions options("abcdefgABCDEFG");
  UnaryStringBenchmark(state, "ascii_trim", &options);
}

#ifdef ARROW_WITH_UTF8PROC
static void Utf8Upper(benchmark::State& state) {
  UnaryStringBenchmark(state, "utf8_upper");
}

static void Utf8Lower(benchmark::State& state) {
  UnaryStringBenchmark(state, "utf8_lower");
}

static void IsAlphaNumericUnicode(benchmark::State& state) {
  UnaryStringBenchmark(state, "utf8_is_alnum");
}
static void TrimSingleUtf8(benchmark::State& state) {
  TrimOptions options("a");
  UnaryStringBenchmark(state, "utf8_trim", &options);
}

static void TrimManyUtf8(benchmark::State& state) {
  TrimOptions options("abcdefgABCDEFG");
  UnaryStringBenchmark(state, "utf8_trim", &options);
}
#endif

BENCHMARK(AsciiLower);
BENCHMARK(AsciiUpper);
BENCHMARK(IsAlphaNumericAscii);
BENCHMARK(MatchSubstring);
BENCHMARK(SplitPattern);
BENCHMARK(TrimSingleAscii);
BENCHMARK(TrimManyAscii);
#ifdef ARROW_WITH_UTF8PROC
BENCHMARK(Utf8Lower);
BENCHMARK(Utf8Upper);
BENCHMARK(IsAlphaNumericUnicode);
BENCHMARK(TrimSingleUtf8);
BENCHMARK(TrimManyUtf8);
#endif

}  // namespace compute
}  // namespace arrow
