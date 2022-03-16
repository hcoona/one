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

#include <cassert>
#include <cstddef>
#include <list>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace hcoona {

template <typename Key, typename Value>
class Lru {
 public:
  explicit Lru(size_t capacity) : capacity_(capacity) { assert(capacity > 0); }

  std::optional<Value> Get(const Key& key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
      return std::nullopt;
    }

    list_.splice(list_.begin(), list_, it->second);
    return list_.front().second;
  }

  template <typename Payload>
  void Put(const Key& key, Payload&& value) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      list_.splice(list_.begin(), list_, it->second);
      list_.front().second = std::forward<Payload>(value);
      return;
    }

    if (size_ == capacity_) {
      map_.erase(list_.back().first);
      list_.pop_back();
      size_--;
    }
    list_.emplace_front(key, std::forward<Payload>(value));
    map_.emplace(key, list_.begin());
    size_++;
  }

 private:
  size_t capacity_;
  size_t size_{0};

  std::list<std::pair<Key, Value>> list_;
  std::unordered_map<Key, typename decltype(list_)::iterator> map_;
};

}  // namespace hcoona
