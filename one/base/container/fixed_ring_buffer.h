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
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>

#include "glog/logging.h"

namespace hcoona {

template <typename T, size_t kCapacity>
class FixedRingBuffer;

namespace details {

// https://isocpp.org/wiki/faq/templates#template-friends
template <typename T, size_t kCapacity, bool kIsConst>
class FixedRingBufferIterator;  // pre-declare the template class itself
template <typename T, size_t kCapacity, bool kIsConst>
bool operator==(const FixedRingBufferIterator<T, kCapacity, kIsConst>& lhs,
                const FixedRingBufferIterator<T, kCapacity, kIsConst>& rhs);

// Meets https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator.
// TODO(zhangshuai.ds): implement the methods.
// https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
template <typename T, size_t kCapacity, bool kIsConst>
class FixedRingBufferIterator {
  using owner_type =
      typename std::conditional<kIsConst, const FixedRingBuffer<T, kCapacity>*,
                                FixedRingBuffer<T, kCapacity>*>::type;

 public:
  using difference_type = size_t;
  using value_type = typename std::conditional<kIsConst, const T, T>::type;
  using pointer = typename std::conditional<kIsConst, const T*, T*>::type;
  using const_pointer = const T*;
  using reference = typename std::conditional<kIsConst, const T&, T&>::type;
  using const_reference = const T&;
  using iterator_category = std::random_access_iterator_tag;

  FixedRingBufferIterator() = default;
  constexpr explicit FixedRingBufferIterator(owner_type buffer) noexcept
      : buffer_(buffer), current_index_(buffer->begin_index_) {}
  constexpr FixedRingBufferIterator(owner_type buffer,
                                    size_t current_index) noexcept
      : buffer_(buffer), current_index_(current_index) {}
  ~FixedRingBufferIterator() = default;

  // Allow copy.
  constexpr FixedRingBufferIterator(const FixedRingBufferIterator&) noexcept =
      default;
  constexpr FixedRingBufferIterator& operator=(
      const FixedRingBufferIterator&) noexcept = default;

  // Allow move.
  constexpr FixedRingBufferIterator(FixedRingBufferIterator&&) noexcept =
      default;
  constexpr FixedRingBufferIterator& operator=(
      FixedRingBufferIterator&&) noexcept = default;

  void swap(FixedRingBufferIterator& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(current_index_, other.current_index_);
  }

  constexpr reference operator*() {
    return buffer_->operator[](current_index_);
  }
  constexpr const_reference operator*() const {
    return buffer_->operator[](current_index_);
  }

  constexpr FixedRingBufferIterator& operator++() {
    current_index_++;
    if (current_index_ == kCapacity) {
      current_index_ = 0;
    }
    return *this;
  }

  constexpr FixedRingBufferIterator& operator++(int) {
    auto it = *this;
    current_index_++;
    if (current_index_ == kCapacity) {
      current_index_ = 0;
    }
    return it;
  }

  constexpr FixedRingBufferIterator& operator--() {
    if (current_index_ == 0) {
      current_index_ = kCapacity - 1;
    } else {
      current_index_--;
    }
    return *this;
  }

  constexpr FixedRingBufferIterator& operator--(int) {
    auto it = *this;
    if (current_index_ == 0) {
      current_index_ = kCapacity - 1;
    } else {
      current_index_--;
    }
    return it;
  }

 private:
  owner_type buffer_{nullptr};
  size_t current_index_{0};

  friend bool operator==
      <>(const FixedRingBufferIterator<T, kCapacity, kIsConst>& lhs,
         const FixedRingBufferIterator<T, kCapacity, kIsConst>& rhs);
};

}  // namespace details

// As `std::queue` required, it meets
// https://en.cppreference.com/w/cpp/named_req/SequenceContainer, without
// support `emplace`/`insert`/`erase`/`assign`. And in addition provide the
// following methods:
// * `back()`
// * `front()`
// * `push_back()`/`emplace_back()`
// * `pop_front()`
template <typename T, size_t kCapacity>
class FixedRingBuffer {
  static_assert(kCapacity > 0, "Capacity cannot be zero.");

 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using iterator =
      details::FixedRingBufferIterator<T, kCapacity, /*kIsConst=*/false>;
  using const_iterator =
      details::FixedRingBufferIterator<T, kCapacity, /*kIsConst=*/true>;
  using difference_type = size_t;
  using size_type = size_t;

  constexpr FixedRingBuffer() noexcept = default;
  constexpr explicit FixedRingBuffer(size_t count) : size_(count) {
    CHECK_LE(count, max_size());
  }
  constexpr FixedRingBuffer(size_t count, const T& value)
      : FixedRingBuffer(count) {
    std::fill_n(begin(), count, value);
  }

  template <typename InputIt>
  constexpr FixedRingBuffer(InputIt begin, InputIt end) {
    while (begin != end) {
      CHECK(!full());
      storage_[size_] = *begin;
      begin++;
      size_++;
    }
  }

  constexpr FixedRingBuffer(std::initializer_list<T> ilist)
      : FixedRingBuffer(ilist.begin(), ilist.end()) {}

  ~FixedRingBuffer() = default;

  // Allow copy.
  constexpr FixedRingBuffer(const FixedRingBuffer&) noexcept = default;
  constexpr FixedRingBuffer& operator=(const FixedRingBuffer&) noexcept =
      default;

  // Allow move.
  constexpr FixedRingBuffer(FixedRingBuffer&&) noexcept = default;
  constexpr FixedRingBuffer& operator=(FixedRingBuffer&&) noexcept = default;

  constexpr FixedRingBuffer& operator=(std::initializer_list<T> ilist) {
    CHECK_LE(ilist.size(), max_size());
    begin_index_ = 0;
    size_ = 0;
    for (const T& v : ilist) {
      storage_[size_] = v;
      size_++;
    }
  }

  void swap(FixedRingBuffer& other) noexcept {
    std::swap(storage_, other.storage_);
    std::swap(begin_index_, other.begin_index_);
    std::swap(size_, other.size_);
  }

  constexpr iterator begin() noexcept { return iterator(this); }
  constexpr const_iterator begin() const noexcept {
    return const_iterator(this);
  }
  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(this);
  }

  constexpr iterator end() noexcept {
    return iterator(this, ToStorageIndex(size_));
  }
  constexpr const_iterator end() const noexcept {
    return const_iterator(this, ToStorageIndex(size_));
  }
  constexpr const_iterator cend() const noexcept {
    return const_iterator(this, ToStorageIndex(size_));
  }

  [[nodiscard]] constexpr size_t size() const noexcept { return size_; }
  [[nodiscard]] constexpr size_t max_size() const noexcept { return kCapacity; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
  [[nodiscard]] constexpr bool full() const noexcept {
    return size() == max_size();
  }

  constexpr void clear() noexcept {
    begin_index_ = 0;
    size_ = 0;
  }

  reference front() { return operator[](0); }
  const_reference front() const { return operator[](0); }

  reference back() {
    CHECK(!empty());
    return operator[](size_ - 1);
  }
  const_reference back() const {
    CHECK(!empty());
    return operator[](size_ - 1);
  }

  template <class... Args>
  constexpr reference emplace_front(Args&&... args) {
    CHECK(!full());
    if (begin_index_ == 0) {
      begin_index_ = kCapacity - 1;
    } else {
      begin_index_--;
    }
    size_++;
    return storage_[begin_index_] = T(std::forward<Args>(args)...);
  }

  constexpr reference push_front(const T& value) {
    return emplace_front(value);
  }
  constexpr reference push_front(T&& value) {
    return emplace_front(std::forward<T>(value));
  }

  template <class... Args>
  reference emplace_back(Args&&... args) {
    CHECK(!full());
    size_++;
    return storage_[ToStorageIndex(size_ - 1)] = T(std::forward<Args>(args)...);
  }

  constexpr reference push_back(const T& value) { return emplace_back(value); }
  constexpr reference push_back(T&& value) { return emplace_back(value); }

  constexpr void pop_front() {
    CHECK(!empty());
    size_--;
    if (begin_index_ == kCapacity - 1) {
      begin_index_ = 0;
    } else {
      begin_index_++;
    }
  }
  constexpr void pop_back() {
    CHECK(!empty());
    size_--;
  }

  constexpr reference operator[](size_t pos) {
    return storage_[ToStorageIndex(pos)];
  }
  constexpr const_reference operator[](size_t pos) const {
    return storage_[ToStorageIndex(pos)];
  }

 private:
  [[nodiscard]] constexpr size_t ToStorageIndex(size_t pos) const {
    CHECK_LE(pos, size_);
    return (begin_index_ + pos) % kCapacity;
  }

  // TODO(zhangshuai.ustc): consider use `kCapacity + 1` for boundary detection.
  T storage_[kCapacity];
  size_t begin_index_{0};
  size_t size_{0};

  template <typename, size_t, bool>
  friend class details::FixedRingBufferIterator;
};

template <typename T, size_t kCapacity>
inline bool operator==(const FixedRingBuffer<T, kCapacity>& lhs,
                       const FixedRingBuffer<T, kCapacity>& rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, size_t kCapacity>
inline bool operator!=(const FixedRingBuffer<T, kCapacity>& lhs,
                       const FixedRingBuffer<T, kCapacity>& rhs) {
  return !(lhs == rhs);
}

template <typename T, size_t kCapacity>
inline void swap(FixedRingBuffer<T, kCapacity>& lhs,
                 FixedRingBuffer<T, kCapacity>& rhs) {
  lhs.swap(rhs);
}

namespace details {

template <typename T, size_t kCapacity, bool kIsConst>
bool operator==(const FixedRingBufferIterator<T, kCapacity, kIsConst>& lhs,
                const FixedRingBufferIterator<T, kCapacity, kIsConst>& rhs) {
  return lhs.buffer_ == rhs.buffer_ && lhs.current_index_ == rhs.current_index_;
}

template <typename T, size_t kCapacity, bool kIsConst>
inline bool operator!=(
    const FixedRingBufferIterator<T, kCapacity, kIsConst>& lhs,
    const FixedRingBufferIterator<T, kCapacity, kIsConst>& rhs) {
  return !(lhs == rhs);
}

template <typename T, size_t kCapacity, bool kIsConst>
inline void swap(FixedRingBufferIterator<T, kCapacity, kIsConst>& lhs,
                 FixedRingBufferIterator<T, kCapacity, kIsConst>& rhs) {
  lhs.swap(rhs);
}

}  // namespace details

}  // namespace hcoona
