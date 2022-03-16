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

#include <cstddef>
#include <initializer_list>
#include <utility>

namespace hcoona {

template <typename T, size_t kCapacity>
class FixedRingBufferIterator {};

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
 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using iterator = FixedRingBufferIterator<T, kCapacity>;
  using const_iterator = const iterator;
  using difference_type = size_t;
  using size_type = size_t;

  FixedRingBuffer() noexcept = default;
  explicit FixedRingBuffer(size_t count);
  FixedRingBuffer(size_t count, const T& value);

  template <typename InputIt>
  FixedRingBuffer(InputIt begin, InputIt end);

  FixedRingBuffer(std::initializer_list<T> ilist);

  ~FixedRingBuffer() = default;

  // Allow copy.
  FixedRingBuffer(const FixedRingBuffer&) noexcept = default;
  FixedRingBuffer& operator=(const FixedRingBuffer&) noexcept = default;

  // Allow move.
  FixedRingBuffer(FixedRingBuffer&&) noexcept = default;
  FixedRingBuffer& operator=(FixedRingBuffer&&) noexcept = default;

  FixedRingBuffer& operator=(std::initializer_list<T> ilist);

  void swap(FixedRingBuffer& other) noexcept;

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  const_iterator cbegin() const noexcept;

  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cend() const noexcept;

  [[nodiscard]] size_t size() const;
  [[nodiscard]] size_t max_size() const { return kCapacity; }
  [[nodiscard]] bool empty() const { size() == 0; }

  void clear() noexcept;

  reference front();
  const_reference front() const;

  reference back();
  const_reference back() const;

  template <class... Args>
  reference emplace_back(Args&&... args);

  void push_back(const T& value);
  void push_back(T&& value);

  void pop_front();
  void pop_back();

  reference operator[](size_type pos);
  const_reference operator[](size_type pos) const;

 private:
  T storage_[kCapacity];
  size_t current_index_{0};
};

template <typename T, size_t kCapacity>
bool operator==(const FixedRingBuffer<T, kCapacity>& lhs,
                const FixedRingBuffer<T, kCapacity>& rhs);

template <typename T, size_t kCapacity>
bool operator!=(const FixedRingBuffer<T, kCapacity>& lhs,
                const FixedRingBuffer<T, kCapacity>& rhs);

}  // namespace hcoona
