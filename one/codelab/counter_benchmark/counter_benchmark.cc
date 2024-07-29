// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.

#include <pthread.h>

#include <atomic>
#include <thread>
#include <vector>

#include "absl/base/internal/per_thread_tls.h"
#include "absl/base/thread_annotations.h"
#include "absl/container/fixed_array.h"
#include "absl/strings/str_format.h"
#include "absl/synchronization/mutex.h"
#include "benchmark/benchmark.h"

class ThreadUnsafeCounter {
 public:
  void Increase() { value_++; }

  int GetAndIncrease() { return value_++; }

  int GetAndDecrease() { return value_--; }

  [[nodiscard]] int Get() const { return value_; }

 private:
  int value_{};
};

class CompareAndSwapCounter {
  static_assert(std::atomic<int>::is_always_lock_free);

 public:
  void Increase() { value_.fetch_add(1, std::memory_order_acq_rel); }

  int GetAndIncrease() {
    return value_.fetch_add(1, std::memory_order_acq_rel);
  }

  int GetAndDecrease() {
    return value_.fetch_sub(1, std::memory_order_acq_rel);
  }

  [[nodiscard]] int Get() const {
    return value_.load(std::memory_order_acquire);
  }

 private:
  std::atomic<int> value_{};
};

class MutexCounter {
 public:
  void Increase() {
    absl::MutexLock lock(&mutex_);
    value_++;
  }

  int GetAndIncrease() {
    absl::MutexLock lock(&mutex_);
    return value_++;
  }

  int GetAndDecrease() {
    absl::MutexLock lock(&mutex_);
    return value_--;
  }

  [[nodiscard]] int Get() const {
    absl::MutexLock lock(&mutex_);
    return value_;
  }

 private:
  mutable absl::Mutex mutex_;
  int value_ ABSL_GUARDED_BY(mutex_){};
};

template <int Threshold>
class PthreadThreadLocalCounter {
 public:
  PthreadThreadLocalCounter() {
    pthread_key_create(&key_, DestroyThreadLocalData);
  }

  PthreadThreadLocalCounter(const PthreadThreadLocalCounter&) = delete;
  PthreadThreadLocalCounter& operator=(const PthreadThreadLocalCounter&) =
      delete;

  PthreadThreadLocalCounter(PthreadThreadLocalCounter&&) = delete;
  PthreadThreadLocalCounter& operator=(PthreadThreadLocalCounter&&) = delete;

  ~PthreadThreadLocalCounter() { pthread_key_delete(key_); }

  void Increase() {
    int* local_value = static_cast<int*>(pthread_getspecific(key_));
    if (local_value == nullptr) {
      // First call in this thread, initialize the value
      local_value = new int(0);
      pthread_setspecific(key_, local_value);
    }

    // Increment the thread local counter
    (*local_value)++;

    if (*local_value >= Threshold) {
      absl::MutexLock lock(&mutex_);
      global_value_ += *local_value;
      *local_value = 0;
    }
  }

  [[nodiscard]] int Get() const {
    absl::MutexLock lock(&mutex_);
    return global_value_;
  }

 private:
  pthread_key_t key_{};

  mutable absl::Mutex mutex_;
  int global_value_ ABSL_GUARDED_BY(mutex_){};

  static void DestroyThreadLocalData(void* ptr) {
    delete static_cast<int*>(ptr);
  }
};

template <typename CounterType>
// NOLINTNEXTLINE(runtime/references)
void BM_IncreasePerThread(benchmark::State& state, int iterationsPerThread) {
  (void)iterationsPerThread;
  for (auto _ : state) {
    CounterType counter;

    absl::FixedArray<std::thread> threads(state.range(0));
    for (auto& thread : threads) {
      thread = std::thread([&counter, iterationsPerThread]() {
        for (int i = 0; i < iterationsPerThread; i++) {
          counter.Increase();
        }
      });
    }

    for (auto& thread : threads) {
      thread.join();
    }

    state.counters["CounterVariance"] =
        iterationsPerThread * state.range(0) - counter.Get();
  }
}

constexpr int kThreadCountMax = 16;

BENCHMARK_TEMPLATE1_CAPTURE(BM_IncreasePerThread, ThreadUnsafeCounter,
                            IterationsPerThread, 1000000)
    ->RangeMultiplier(2)
    ->Range(1, kThreadCountMax)
    ->UseRealTime();

BENCHMARK_TEMPLATE1_CAPTURE(BM_IncreasePerThread, CompareAndSwapCounter,
                            IterationsPerThread, 1000000)
    ->RangeMultiplier(2)
    ->Range(1, kThreadCountMax)
    ->UseRealTime();

BENCHMARK_TEMPLATE1_CAPTURE(BM_IncreasePerThread, MutexCounter,
                            IterationsPerThread, 1000000)
    ->RangeMultiplier(2)
    ->Range(1, kThreadCountMax)
    ->UseRealTime();

BENCHMARK_TEMPLATE1_CAPTURE(BM_IncreasePerThread,
                            PthreadThreadLocalCounter<128>, IterationsPerThread,
                            1000000)
    ->RangeMultiplier(2)
    ->Range(1, kThreadCountMax)
    ->UseRealTime();

BENCHMARK_MAIN();
