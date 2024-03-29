/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <algorithm>
#include <exception>
#include <functional>

#include "boost/intrusive/list.hpp"
#include "boost/intrusive/unordered_set.hpp"
#include "boost/iterator/iterator_adaptor.hpp"
#include "boost/utility.hpp"

#include "folly/container/HeterogeneousAccess.h"
#include "folly/lang/Exception.h"

namespace folly {

/**
 * A general purpose LRU evicting cache. Designed to support constant time
 * set/get operations. It maintains a doubly linked list of items that are
 * threaded through an index (a hash map). The access ordered is maintained
 * on the list by moving an element to the front of list on a get. New elements
 * are added to the front of the list. The index size is set to half the
 * capacity (setting capacity to 0 is a special case. see notes at the end of
 * this section). So assuming uniform distribution of keys, set/get are both
 * constant time operations.
 *
 * On reaching capacity limit, clearSize_ LRU items are evicted at a time. If
 * a callback is specified with setPruneHook, it is invoked for each eviction.
 *
 * This is NOT a thread-safe implementation.
 *
 * Configurability: capacity of the cache, number of items to evict, eviction
 * callback and the hasher to hash the keys can all be supplied by the caller.
 *
 * If at a given state, N1 - N6 are the nodes in MRU to LRU order and hashing
 * to index keys as {(N1,N5)->H1, (N4,N2,N6)->H2, N3->Hi}, the datastructure
 * layout is as below. N1 .. N6 is a list threaded through the hash.
 * Assuming, each the number of nodes hashed to each index key is bounded, the
 * following operations run in constant time.
 * i) get computes the index key, walks the list of elements hashed to
 * the key and moves it to the front of the list, if found.
 * ii) set inserts a new node into the list and places the same node on to the
 * list of elements hashing to the corresponding index key.
 * ii) prune deletes nodes from the end of the list as well from the index.
 *
 * +----+     +----+     +----+
 * | H1 | <-> | N1 | <-> | N5 |
 * +----+     +----+     +----+
 *              ^        ^  ^
 *              |    ___/    \
 *              |   /         \
 *              |_ /________   \___
 *                /        |       \
 *               /         |        \
 *              v          v         v
 * +----+     +----+     +----+     +----+
 * | H2 | <-> | N4 | <-> | N2 | <-> | N6 |
 * +----+     +----+     +----+     +----+
 *   .          ^          ^
 *   .          |          |
 *   .          |          |
 *   .          |     _____|
 *   .          |    /
 *              v   v
 * +----+     +----+
 * | Hi | <-> | N3 |
 * +----+     +----+
 *
 * N.B 1 : Changing the capacity with setMaxSize does not change the index size
 * and it could end up in too many elements indexed to the same slot in index.
 * The set/get performance will get worse in this case. So it is best to avoid
 * resizing.
 *
 * N.B 2 : Setting capacity to 0, using setMaxSize or initialization, turns off
 * evictions based on sizeof the cache making it an INFINITE size cache
 * unless evictions of LRU items are triggered by calling prune() by clients
 * (using their own eviction criteria).
 */
template <
    class TKey,
    class TValue,
    class THash = HeterogeneousAccessHash<TKey>,
    class TKeyEqual = HeterogeneousAccessEqualTo<TKey>>
class EvictingCacheMap {
 private:
  // typedefs for brevity
  struct Node;
  struct KeyHasher;
  struct KeyValueEqual;
  using LinkMode = boost::intrusive::link_mode<boost::intrusive::safe_link>;
  using NodeMap = boost::intrusive::unordered_set<
      Node,
      boost::intrusive::hash<KeyHasher>,
      boost::intrusive::equal<KeyValueEqual>>;
  using NodeList = boost::intrusive::list<Node>;
  using TPair = std::pair<const TKey, TValue>;

 public:
  using PruneHookCall = std::function<void(TKey, TValue&&)>;

  // iterator base : returns TPair on dereference
  template <typename Value, typename TIterator>
  class iterator_base : public boost::iterator_adaptor<
                            iterator_base<Value, TIterator>,
                            TIterator,
                            Value,
                            boost::bidirectional_traversal_tag> {
   public:
    iterator_base() {}

    explicit iterator_base(TIterator it)
        : iterator_base::iterator_adaptor_(it) {}

    template <
        typename V,
        typename I,
        std::enable_if_t<
            std::is_same<V const, Value>::value &&
                std::is_convertible<I, TIterator>::value,
            int> = 0>
    /* implicit */ iterator_base(iterator_base<V, I> const& other)
        : iterator_base::iterator_adaptor_(other.base()) {}

    Value& dereference() const { return this->base_reference()->pr; }
  };

  // iterators
  using iterator = iterator_base<TPair, typename NodeList::iterator>;
  using const_iterator =
      iterator_base<const TPair, typename NodeList::const_iterator>;
  using reverse_iterator =
      iterator_base<TPair, typename NodeList::reverse_iterator>;
  using const_reverse_iterator =
      iterator_base<const TPair, typename NodeList::const_reverse_iterator>;

  // the default map typedefs
  using key_type = TKey;
  using mapped_type = TValue;
  using hasher = THash;

 private:
  template <typename K, typename T>
  using EnableHeterogeneousFind = std::enable_if_t<
      detail::EligibleForHeterogeneousFind<TKey, THash, TKeyEqual, K>::value,
      T>;

  template <typename K, typename T>
  using EnableHeterogeneousInsert = std::enable_if_t<
      detail::EligibleForHeterogeneousInsert<TKey, THash, TKeyEqual, K>::value,
      T>;

  template <typename K>
  using IsIter = Disjunction<
      std::is_same<iterator, remove_cvref_t<K>>,
      std::is_same<const_iterator, remove_cvref_t<K>>>;

  template <typename K, typename T>
  using EnableHeterogeneousErase = std::enable_if_t<
      detail::EligibleForHeterogeneousFind<
          TKey,
          THash,
          TKeyEqual,
          std::conditional_t<IsIter<K>::value, TKey, K>>::value &&
          !IsIter<K>::value,
      T>;

 public:
  /**
   * Construct a EvictingCacheMap
   * @param maxSize maximum size of the cache map.  Once the map size exceeds
   *     maxSize, the map will begin to evict.
   * @param clearSize the number of elements to clear at a time when the
   *     eviction size is reached.
   */
  explicit EvictingCacheMap(
      std::size_t maxSize,
      std::size_t clearSize = 1,
      const THash& keyHash = THash(),
      const TKeyEqual& keyEqual = TKeyEqual())
      : nIndexBuckets_(std::max(maxSize / 2, std::size_t(kMinNumIndexBuckets))),
        indexBuckets_(new typename NodeMap::bucket_type[nIndexBuckets_]),
        indexTraits_(indexBuckets_.get(), nIndexBuckets_),
        keyHash_(keyHash),
        keyEqual_(keyEqual),
        index_(indexTraits_, keyHash_, keyEqual_),
        maxSize_(maxSize),
        clearSize_(clearSize) {}

  EvictingCacheMap(const EvictingCacheMap&) = delete;
  EvictingCacheMap& operator=(const EvictingCacheMap&) = delete;
  EvictingCacheMap(EvictingCacheMap&&) = default;
  EvictingCacheMap& operator=(EvictingCacheMap&&) = default;

  ~EvictingCacheMap() {
    setPruneHook(nullptr);
    // ignore any potential exceptions from pruneHook_
    pruneWithFailSafeOption(size(), nullptr, true);
  }

  /**
   * Adjust the max size of EvictingCacheMap. Note that this does not update
   * nIndexBuckets_ accordingly. This API can cause performance to get very
   * bad, e.g., the nIndexBuckets_ is still 100 after maxSize is updated to 1M.
   *
   * Calling this function with an arugment of 0 removes the limit on the cache
   * size and elements are not evicted unless clients explicitly call prune.
   *
   * If you intend to resize dynamically using this, then picking an index size
   * that works well and initializing with corresponding maxSize is the only
   * reasonable option.
   *
   * @param maxSize new maximum size of the cache map.
   * @param pruneHook callback to use on eviction.
   */
  void setMaxSize(size_t maxSize, PruneHookCall pruneHook = nullptr) {
    if (maxSize != 0 && maxSize < size()) {
      // Prune the excess elements with our new constraints.
      prune(std::max(size() - maxSize, clearSize_), pruneHook);
    }
    maxSize_ = maxSize;
  }

  size_t getMaxSize() const { return maxSize_; }

  void setClearSize(size_t clearSize) { clearSize_ = clearSize; }

  /**
   * Check for existence of a specific key in the map.  This operation has
   *     no effect on LRU order.
   * @param key key to search for
   * @return true if exists, false otherwise
   */
  bool exists(const TKey& key) const { return existsImpl(key); }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  bool exists(const K& key) const {
    return existsImpl(key);
  }

  /**
   * Get the value associated with a specific key.  This function always
   *     promotes a found value to the head of the LRU.
   * @param key key associated with the value
   * @return the value if it exists
   * @throw std::out_of_range exception of the key does not exist
   */
  TValue& get(const TKey& key) { return getImpl(key); }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  TValue& get(const K& key) {
    return getImpl(key);
  }

  /**
   * Get the iterator associated with a specific key.  This function always
   *     promotes a found value to the head of the LRU.
   * @param key key to associate with value
   * @return the iterator of the object (a std::pair of const TKey, TValue) or
   *     end() if it does not exist
   */
  iterator find(const TKey& key) { return findImpl(*this, key); }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  iterator find(const K& key) {
    return findImpl(*this, key);
  }

  /**
   * Get the value associated with a specific key.  This function never
   *     promotes a found value to the head of the LRU.
   * @param key key associated with the value
   * @return the value if it exists
   * @throw std::out_of_range exception of the key does not exist
   */
  const TValue& getWithoutPromotion(const TKey& key) const {
    return getWithoutPromotionImpl(*this, key);
  }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  const TValue& getWithoutPromotion(const K& key) const {
    return getWithoutPromotionImpl(*this, key);
  }

  TValue& getWithoutPromotion(const TKey& key) {
    return getWithoutPromotionImpl(*this, key);
  }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  TValue& getWithoutPromotion(const K& key) {
    return getWithoutPromotionImpl(*this, key);
  }

  /**
   * Get the iterator associated with a specific key.  This function never
   *     promotes a found value to the head of the LRU.
   * @param key key to associate with value
   * @return the iterator of the object (a std::pair of const TKey, TValue) or
   *     end() if it does not exist
   */
  const_iterator findWithoutPromotion(const TKey& key) const {
    return findWithoutPromotionImpl(*this, key);
  }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  const_iterator findWithoutPromotion(const K& key) const {
    return findWithoutPromotionImpl(*this, key);
  }

  iterator findWithoutPromotion(const TKey& key) {
    return findWithoutPromotionImpl(*this, key);
  }

  template <typename K, EnableHeterogeneousFind<K, int> = 0>
  iterator findWithoutPromotion(const K& key) {
    return findWithoutPromotionImpl(*this, key);
  }

  /**
   * Erase the key-value pair associated with key if it exists.
   * @param key key associated with the value
   * @return true if the key existed and was erased, else false
   */
  bool erase(const TKey& key) { return eraseImpl(key); }

  template <typename K, EnableHeterogeneousErase<K, int> = 0>
  bool erase(const K& key) {
    return eraseImpl(key);
  }

  /**
   * Erase the key-value pair associated with pos
   * @param pos iterator to the element to be erased
   * @return iterator to the following element or end() if pos was the last
   *     element
   */
  iterator erase(const_iterator pos) {
    auto* node = const_cast<Node*>(&(*pos.base()));
    std::unique_ptr<Node> nptr(node);
    index_.erase(index_.iterator_to(*node));
    return iterator(lru_.erase(pos.base()));
  }

  /**
   * Set a key-value pair in the dictionary
   * @param key key to associate with value
   * @param value value to associate with the key
   * @param promote boolean flag indicating whether or not to move something
   *     to the front of an LRU.  This only really matters if you're setting
   *     a value that already exists.
   * @param pruneHook callback to use on eviction (if it occurs).
   */
  void set(
      const TKey& key,
      TValue value,
      bool promote = true,
      PruneHookCall pruneHook = nullptr) {
    setImpl(key, std::forward<TValue>(value), promote, pruneHook);
  }

  template <typename K, EnableHeterogeneousInsert<K, int> = 0>
  void set(
      const K& key,
      TValue value,
      bool promote = true,
      PruneHookCall pruneHook = nullptr) {
    setImpl(key, std::forward<TValue>(value), promote, pruneHook);
  }

  /**
   * Insert a new key-value pair in the dictionary if no element exists for key
   * @param key key to associate with value
   * @param value value to associate with the key
   * @param pruneHook callback to use on eviction (if it occurs).
   * @return a pair consisting of an iterator to the inserted element (or to the
   *     element that prevented the insertion) and a bool denoting whether the
   *     insertion took place.
   */
  std::pair<iterator, bool> insert(
      const TKey& key, TValue value, PruneHookCall pruneHook = nullptr) {
    return insertImpl(key, std::forward<TValue>(value), pruneHook);
  }

  template <typename K, EnableHeterogeneousInsert<K, int> = 0>
  std::pair<iterator, bool> insert(
      const K& key, TValue value, PruneHookCall pruneHook = nullptr) {
    return insertImpl(key, std::forward<TValue>(value), pruneHook);
  }

  /**
   * Get the number of elements in the dictionary
   * @return the size of the dictionary
   */
  std::size_t size() const { return index_.size(); }

  /**
   * Typical empty function
   * @return true if empty, false otherwise
   */
  bool empty() const { return index_.empty(); }

  void clear(PruneHookCall pruneHook = nullptr) { prune(size(), pruneHook); }

  /**
   * Set the prune hook, which is the function invoked on the key and value
   *     on each eviction.  Will throw If the pruneHook throws, unless the
   *     EvictingCacheMap object is being destroyed in which case it will
   *     be ignored.
   * @param pruneHook new callback to use on eviction.
   * @param promote boolean flag indicating whether or not to move something
   *     to the front of an LRU.
   * @return the iterator of the object (a std::pair of const TKey, TValue) or
   *     end() if it does not exist
   */
  void setPruneHook(PruneHookCall pruneHook) { pruneHook_ = pruneHook; }

  /**
   * Prune the minimum of pruneSize and size() from the back of the LRU.
   * Will throw if pruneHook throws.
   * @param pruneSize minimum number of elements to prune
   * @param pruneHook a custom pruneHook function
   */
  void prune(std::size_t pruneSize, PruneHookCall pruneHook = nullptr) {
    // do not swallow exceptions for prunes not triggered from destructor
    pruneWithFailSafeOption(pruneSize, pruneHook, false);
  }

  // Iterators and such
  iterator begin() { return iterator(lru_.begin()); }
  iterator end() { return iterator(lru_.end()); }
  const_iterator begin() const { return const_iterator(lru_.begin()); }
  const_iterator end() const { return const_iterator(lru_.end()); }

  const_iterator cbegin() const { return const_iterator(lru_.cbegin()); }
  const_iterator cend() const { return const_iterator(lru_.cend()); }

  reverse_iterator rbegin() { return reverse_iterator(lru_.rbegin()); }
  reverse_iterator rend() { return reverse_iterator(lru_.rend()); }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(lru_.rbegin());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(lru_.rend());
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(lru_.crbegin());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(lru_.crend());
  }

 private:
  struct Node : public boost::intrusive::unordered_set_base_hook<LinkMode>,
                public boost::intrusive::list_base_hook<LinkMode> {
    template <typename K>
    Node(const K& key, TValue&& value) : pr(key, std::move(value)) {}
    TPair pr;
  };

  struct KeyHasher {
    KeyHasher(const THash& keyHash) : hash(keyHash) {}
    std::size_t operator()(const Node& node) const {
      return hash(node.pr.first);
    }
    template <typename K>
    std::size_t operator()(const K& key) const {
      return hash(key);
    }
    THash hash;
  };

  struct KeyValueEqual {
    KeyValueEqual(const TKeyEqual& keyEqual) : equal(keyEqual) {}
    template <typename K>
    bool operator()(const K& lhs, const Node& rhs) const {
      return equal(lhs, rhs.pr.first);
    }
    template <typename K>
    bool operator()(const Node& lhs, const K& rhs) const {
      return equal(lhs.pr.first, rhs);
    }
    bool operator()(const Node& lhs, const Node& rhs) const {
      return equal(lhs.pr.first, rhs.pr.first);
    }
    TKeyEqual equal;
  };

  template <typename K>
  bool existsImpl(const K& key) const {
    return findInIndex(key) != index_.end();
  }

  template <typename K>
  TValue& getImpl(const K& key) {
    auto it = findImpl(*this, key);
    if (it == end()) {
      throw_exception<std::out_of_range>("Key does not exist");
    }
    return it->second;
  }

  template <typename Self>
  using self_iterator_t =
      std::conditional_t<std::is_const<Self>::value, const_iterator, iterator>;

  template <typename Self, typename K>
  static auto findImpl(Self& self, const K& key) {
    auto it = self.findInIndex(key);
    if (it == self.index_.end()) {
      return self.end();
    }
    self.lru_.splice(self.lru_.begin(), self.lru_, self.lru_.iterator_to(*it));
    return self_iterator_t<Self>(self.lru_.iterator_to(*it));
  }

  template <typename Self, typename K>
  static auto& getWithoutPromotionImpl(Self& self, const K& key) {
    auto it = self.findWithoutPromotion(key);
    if (it == self.end()) {
      throw_exception<std::out_of_range>("Key does not exist");
    }
    return it->second;
  }

  template <typename Self, typename K>
  static auto findWithoutPromotionImpl(Self& self, const K& key) {
    auto it = self.findInIndex(key);
    return (it == self.index_.end())
        ? self.end()
        : self_iterator_t<Self>(self.lru_.iterator_to(*it));
  }

  template <typename K>
  bool eraseImpl(const K& key) {
    auto it = findInIndex(key);
    if (it != index_.end()) {
      erase(const_iterator(lru_.iterator_to(*it)));
      return true;
    }
    return false;
  }

  template <typename K>
  void setImpl(
      const K& key, TValue value, bool promote, PruneHookCall pruneHook) {
    auto it = findInIndex(key);
    if (it != index_.end()) {
      it->pr.second = std::move(value);
      if (promote) {
        lru_.splice(lru_.begin(), lru_, lru_.iterator_to(*it));
      }
    } else {
      auto node = new Node(key, std::move(value));
      index_.insert(*node);
      lru_.push_front(*node);

      // no evictions if maxSize_ is 0 i.e. unlimited capacity
      if (maxSize_ > 0 && size() > maxSize_) {
        prune(clearSize_, pruneHook);
      }
    }
  }

  template <typename K>
  auto insertImpl(const K& key, TValue value, PruneHookCall pruneHook) {
    auto node = std::make_unique<Node>(key, std::move(value));
    auto pair = index_.insert(*node);
    if (pair.second) {
      lru_.push_front(*node);
      node.release();

      // no evictions if maxSize_ is 0 i.e. unlimited capacity
      if (maxSize_ > 0 && size() > maxSize_) {
        prune(clearSize_, pruneHook);
      }
    }
    return std::pair<iterator, bool>(
        lru_.iterator_to(*pair.first), pair.second);
  }

  /**
   * Get the iterator in in the index associated with a specific key. This is
   * merely a search in the index and does not promote the object.
   * @param key key to associate with value
   * @return the NodeMap::iterator to the Node containing the object
   *    (a std::pair of const TKey, TValue) or index_.end() if it does not exist
   */
  template <typename K>
  typename NodeMap::iterator findInIndex(const K& key) {
    return index_.find(key, KeyHasher(keyHash_), KeyValueEqual(keyEqual_));
  }

  template <typename K>
  typename NodeMap::const_iterator findInIndex(const K& key) const {
    return index_.find(key, KeyHasher(keyHash_), KeyValueEqual(keyEqual_));
  }

  /**
   * Prune the minimum of pruneSize and size() from the back of the LRU.
   * @param pruneSize minimum number of elements to prune
   * @param pruneHook a custom pruneHook function
   * @param failSafe true if exceptions are to ignored, false by default
   */
  void pruneWithFailSafeOption(
      std::size_t pruneSize, PruneHookCall pruneHook, bool failSafe) {
    auto& ph = (nullptr == pruneHook) ? pruneHook_ : pruneHook;

    for (std::size_t i = 0; i < pruneSize && !lru_.empty(); i++) {
      auto* node = &(*lru_.rbegin());
      std::unique_ptr<Node> nptr(node);

      lru_.erase(lru_.iterator_to(*node));
      index_.erase(index_.iterator_to(*node));
      if (ph) {
        try {
          ph(node->pr.first, std::move(node->pr.second));
        } catch (...) {
          if (!failSafe) {
            throw;
          }
        }
      }
    }
  }

  static const std::size_t kMinNumIndexBuckets = 100;
  PruneHookCall pruneHook_;
  std::size_t nIndexBuckets_;
  std::unique_ptr<typename NodeMap::bucket_type[]> indexBuckets_;
  typename NodeMap::bucket_traits indexTraits_;
  THash keyHash_;
  TKeyEqual keyEqual_;
  NodeMap index_;
  NodeList lru_;
  std::size_t maxSize_;
  std::size_t clearSize_;
};

} // namespace folly
