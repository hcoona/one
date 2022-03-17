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

#pragma once

#include <algorithm>
#include <optional>
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "one/base/container/fixed_ring_buffer.h"

namespace hcoona {

template <typename T, size_t kCapacity>
class BlockingBoundedQueue final {
 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using difference_type = size_t;
  using size_type = size_t;

  BlockingBoundedQueue() = default;
  ~BlockingBoundedQueue() = default;

  // Allow copy but not implemented yet.
  BlockingBoundedQueue(const BlockingBoundedQueue&) noexcept = delete;
  BlockingBoundedQueue& operator=(const BlockingBoundedQueue&) noexcept =
      delete;

  // Allow move but not implemented yet.
  BlockingBoundedQueue(BlockingBoundedQueue&&) noexcept = delete;
  BlockingBoundedQueue& operator=(BlockingBoundedQueue&&) noexcept = delete;

  //
  // Capacity
  //

  [[nodiscard]] size_t size() const noexcept {
    absl::ReaderMutexLock lock(&mutex_);
    return buffer_.size();
  }
  [[nodiscard]] size_t max_size() const noexcept {
    absl::ReaderMutexLock lock(&mutex_);
    return buffer_.max_size();
  }
  [[nodiscard]] bool empty() const noexcept {
    absl::ReaderMutexLock lock(&mutex_);
    return buffer_.empty();
  }
  [[nodiscard]] bool full() const noexcept {
    absl::ReaderMutexLock lock(&mutex_);
    return buffer_.full();
  }

  //
  // Modifiers
  //

  void clear() {
    absl::WriterMutexLock lock(&mutex_);
    buffer_.clear();
    not_full_cv_.SignalAll();
  }

  template <typename OutputIt>
  void clear(OutputIt dest_first) {
    absl::WriterMutexLock lock(&mutex_);
    std::move(buffer_.begin(), buffer_.end(), dest_first);
    buffer_.clear();
    not_full_cv_.SignalAll();
  }

  template <typename... Args>
  reference emplace_front(Args&&... args) {
    absl::WriterMutexLock lock(&mutex_);
    while (buffer_.full()) {
      not_full_cv_.Wait(&mutex_);
    }
    reference result = buffer_.emplace_front(std::forward<Args>(args)...);
    not_empty_cv_.Signal();
    return result;
  }

  template <typename... Args>
  reference emplace_back(Args&&... args) {
    absl::WriterMutexLock lock(&mutex_);
    while (buffer_.full()) {
      not_full_cv_.Wait(&mutex_);
    }
    reference result = buffer_.emplace_back(std::forward<Args>(args)...);
    not_empty_cv_.Signal();
    return result;
  }

  //
  // Taking elements
  //

  value_type PopFrontAndReturn() {
    absl::WriterMutexLock lock(&mutex_);
    while (buffer_.empty()) {
      not_empty_cv_.Wait(&mutex_);
    }
    value_type result = std::move(buffer_.front());
    buffer_.pop_front();
    not_full_cv_.Signal();
    return result;
  }

  std::optional<value_type> TryPopFrontAndReturn() {
    absl::WriterMutexLock lock(&mutex_);
    if (buffer_.empty()) {
      return std::nullopt;
    }

    value_type result = std::move(buffer_.front());
    buffer_.pop_front();
    not_full_cv_.Signal();
    return result;
  }

  value_type PopBackAndReturn() {
    absl::WriterMutexLock lock(&mutex_);
    while (buffer_.empty()) {
      not_empty_cv_.Wait(&mutex_);
    }
    value_type result = std::move(buffer_.back());
    buffer_.pop_back();
    not_full_cv_.Signal();
    return result;
  }

  std::optional<value_type> TryPopBackAndReturn() {
    absl::WriterMutexLock lock(&mutex_);
    if (buffer_.empty()) {
      return std::nullopt;
    }

    value_type result = std::move(buffer_.back());
    buffer_.pop_back();
    not_full_cv_.Signal();
    return result;
  }

  // TODO(zhangshuai.ustc): PopFrontN & PopBackN.

 private:
  mutable absl::Mutex mutex_;
  absl::CondVar not_empty_cv_ ABSL_GUARDED_BY(mutex_);
  absl::CondVar not_full_cv_ ABSL_GUARDED_BY(mutex_);
  FixedRingBuffer<T, kCapacity> buffer_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace hcoona
