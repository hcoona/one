#include <array>
#include <limits>

#include "absl/random/random.h"
#include "absl/strings/numbers.h"
#include "benchmark/benchmark.h"
#include "glog/logging.h"

namespace {

void TestAbslAtoi(benchmark::State& state) {  // NOLINT
  static constexpr const int kIterations = 1 << 12;

  absl::BitGen bitgen_;
  std::array<int64_t, kIterations> expected_numbers;
  std::array<std::string, kIterations> expected_numbers_strings;
  for (int i = 0; i < kIterations; i++) {
    int64_t expected =
        absl::Uniform(bitgen_, std::numeric_limits<int64_t>::min(),
                      std::numeric_limits<int64_t>::max());
    expected_numbers[i] = expected;
    expected_numbers_strings[i] = std::to_string(expected);
  }

  for (auto _ : state) {
    for (int i = 0; i < kIterations; i++) {
      int64_t expected = expected_numbers[i];
      int64_t actual;
      CHECK(absl::SimpleAtoi(expected_numbers_strings[i], &actual));
      CHECK_EQ(actual, expected);
    }
    state.SetItemsProcessed(kIterations);
  }
}

void TestStdAtoi(benchmark::State& state) {  // NOLINT
  static constexpr const int kIterations = 1 << 12;

  absl::BitGen bitgen_;
  std::array<int64_t, kIterations> expected_numbers;
  std::array<std::string, kIterations> expected_numbers_strings;
  for (int i = 0; i < kIterations; i++) {
    int64_t expected =
        absl::Uniform(bitgen_, std::numeric_limits<int64_t>::min(),
                      std::numeric_limits<int64_t>::max());
    expected_numbers[i] = expected;
    expected_numbers_strings[i] = std::to_string(expected);
  }

  for (auto _ : state) {
    for (int i = 0; i < kIterations; i++) {
      int64_t expected = expected_numbers[i];
      int64_t actual = std::stoll(expected_numbers_strings[i]);
      CHECK_EQ(expected, actual);
    }
    state.SetItemsProcessed(kIterations);
  }
}

BENCHMARK(TestAbslAtoi);
BENCHMARK(TestStdAtoi);

}  // namespace
