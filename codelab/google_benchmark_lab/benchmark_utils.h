#ifndef CODELAB_GOOGLE_BENCHMARK_LAB_BENCHMARK_UTILS_H_
#define CODELAB_GOOGLE_BENCHMARK_LAB_BENCHMARK_UTILS_H_

#include "benchmark/benchmark.h"

namespace codelab {

class ScopedPauseTiming {
 public:
  explicit ScopedPauseTiming(benchmark::State& state) : state_(state) {
    state_.PauseTiming();
  }
  ScopedPauseTiming(const ScopedPauseTiming&) = delete;
  ScopedPauseTiming& operator=(const ScopedPauseTiming&) = delete;
  ~ScopedPauseTiming() { state_.ResumeTiming(); }

 private:
  benchmark::State& state_;
};

class ScopedResumeTiming {
 public:
  explicit ScopedResumeTiming(benchmark::State& state) : state_(state) {
    state_.ResumeTiming();
  }
  ScopedResumeTiming(const ScopedResumeTiming&) = delete;
  ScopedResumeTiming& operator=(const ScopedResumeTiming&) = delete;
  ~ScopedResumeTiming() { state_.PauseTiming(); }

 private:
  benchmark::State& state_;
};

}  // namespace codelab

#endif  // CODELAB_GOOGLE_BENCHMARK_LAB_BENCHMARK_UTILS_H_
