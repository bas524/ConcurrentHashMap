#pragma once

#include <shared_mutex>
#include <algorithm>
#include <unordered_map>

template <typename F, typename Mtx>
auto DefenceScopeReadLock(F f, Mtx &mtx, bool withLock) -> typename std::enable_if<std::is_same<decltype(f()), void>::value, void>::type {
  if (withLock) {
    mtx.lock_shared();
  }
  try {
    f();
  } catch (...) {
    if (withLock) {
      mtx.unlock_shared();
    }
    throw;
  }
  if (withLock) {
    mtx.unlock_shared();
  }
}
template <typename F, typename Mtx>
auto DefenceScopeWriteLock(F f, Mtx &mtx, bool withLock) -> typename std::enable_if<std::is_same<decltype(f()), void>::value, void>::type {
  if (withLock) {
    mtx.lock();
  }
  try {
    f();
  } catch (...) {
    if (withLock) {
      mtx.unlock();
    }
    throw;
  }
  if (withLock) {
    mtx.unlock();
  }
}
template <typename F, typename Mtx>
auto DefenceScopeReadLock(F f, Mtx &mtx, bool withLock) -> typename std::enable_if<!std::is_same<decltype(f()), void>::value, decltype(f())>::type {
  decltype(f()) result;
  if (withLock) {
    mtx.lock_shared();
  }
  try {
    result = f();
  } catch (...) {
    if (withLock) {
      mtx.unlock_shared();
    }
    throw;
  }
  if (withLock) {
    mtx.unlock_shared();
  }
  return result;
}
template <typename F, typename Mtx>
auto DefenceScopeWriteLock(F f, Mtx &mtx, bool withLock) -> typename std::enable_if<!std::is_same<decltype(f()), void>::value, decltype(f())>::type {
  decltype(f()) result;
  if (withLock) {
    mtx.lock();
  }
  try {
    result = f();
  } catch (...) {
    if (withLock) {
      mtx.unlock();
    }
    throw;
  }
  if (withLock) {
    mtx.unlock();
  }
  return result;
}

template <typename Key, typename Value>
class ConcurrentHashMap {
  mutable std::unordered_map<Key, Value> _map;
  mutable std::shared_timed_mutex _rwLock;

 public:
  enum class OperationMode { WITH_LOCK, FORCE_NO_LOCK };
  using ValueType = typename std::unordered_map<Key, Value>::value_type;
  ConcurrentHashMap() = default;
  explicit ConcurrentHashMap(size_t n) : _map(n) {}
  ConcurrentHashMap(std::initializer_list<ValueType> list, size_t n = 0) : _map(list, n) {}
  ConcurrentHashMap(const ConcurrentHashMap &o) {
    std::shared_lock<std::shared_timed_mutex> readRWLock(o._rwLock);
    _map = o._map;
  }
  ConcurrentHashMap(ConcurrentHashMap &&o) noexcept : _rwLock() {
    std::unique_lock<std::shared_timed_mutex> writeRWLock(o._rwLock);
    _map = std::move(o._map);
  }
  const std::shared_timed_mutex &rwLock() const { return _rwLock; }
  std::shared_timed_mutex &rwLock() { return _rwLock; }
  void insert(const Key &key, const Value &value, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock([&]() { _map.insert(std::make_pair(key, value)); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  void emplace(Key &&key, Value &&value, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock([&]() { _map.emplace(std::forward<Key>(key), std::forward<Value>(value)); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  bool empty(OperationMode mode = OperationMode::WITH_LOCK) const {
    return DefenceScopeReadLock([&]() -> bool { return _map.empty(); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  size_t size(OperationMode mode = OperationMode::WITH_LOCK) const {
    return DefenceScopeReadLock([&]() -> size_t { return _map.size(); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  bool contains(const Key &k, OperationMode mode = OperationMode::WITH_LOCK) const {
    return DefenceScopeReadLock([&]() -> bool { _map.find(k) != _map.end(); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  void erase(const Key &key, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock([&]() { _map.erase(key); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  void eraseKeys(const std::vector<Key> &keys, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock(
        [&]() {
          for (const auto &k : keys) {
            _map.erase(k);
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Predicate>
  void eraseIf(Predicate predicate, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock(
        [&]() {
          for (auto it = _map.begin(); it != _map.end();) {
            if (predicate(*it)) {
              it = _map.erase(it);
            } else {
              ++it;
            }
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Predicate>
  void eraseKeyIf(Predicate predicate, const Key &key, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock(
        [&]() {
          auto item = _map.find(key);
          if (item != _map.end() && predicate(*item)) {
            _map.erase(item);
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Function>
  void doForEeach(Function function, OperationMode mode = OperationMode::WITH_LOCK) const {
    DefenceScopeReadLock([&]() { std::for_each(_map.begin(), _map.end(), function); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  template <typename Function>
  void applyForEeach(Function function, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock([&]() { std::for_each(_map.begin(), _map.end(), function); }, _rwLock, mode == OperationMode::WITH_LOCK);
  }
  template <typename Function, typename Predicate>
  void doForEeachIf(Function function, Predicate predicate, OperationMode mode = OperationMode::WITH_LOCK) const {
    DefenceScopeReadLock(
        [&]() {
          for (const auto &item : _map) {
            if (predicate(item)) {
              function(item);
            }
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Function, typename Predicate>
  void applyForEeachIf(Function function, Predicate predicate, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock(
        [&]() {
          for (auto &item : _map) {
            if (predicate(item)) {
              function(item);
            }
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Function>
  void doForKey(Function function, const Key &key, OperationMode mode = OperationMode::WITH_LOCK) const {
    DefenceScopeReadLock(
        [&]() {
          auto item = _map.find(key);
          if (item != _map.end()) {
            function(*item);
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Function>
  void applyForKey(Function function, const Key &key, OperationMode mode = OperationMode::WITH_LOCK) {
    DefenceScopeWriteLock(
        [&]() {
          auto item = _map.find(key);
          if (item != _map.end()) {
            function(*item);
          }
        },
        _rwLock,
        mode == OperationMode::WITH_LOCK);
  }
  template <typename Function>
  void insertOrDoIfExists(const Key &key, const Value &value, Function f) {
    if (contains(key)) {
      doForKey(f, key);
    } else {
      std::unique_lock<std::shared_timed_mutex> writeRWLock(_rwLock);
      if (contains(key, OperationMode::FORCE_NO_LOCK)) {
        doForKey(f, key, OperationMode::FORCE_NO_LOCK);
      } else {
        insert(key, value, OperationMode::FORCE_NO_LOCK);
      }
    }
  }
  template <typename Function>
  void emplaceOrDoIfExists(Key &&key, Value &&value, Function f) {
    if (contains(key)) {
      doForKey(f, key);
    } else {
      std::unique_lock<std::shared_timed_mutex> writeRWLock(_rwLock);
      if (contains(key, OperationMode::FORCE_NO_LOCK)) {
        doForKey(f, key, OperationMode::FORCE_NO_LOCK);
      } else {
        emplace(std::move(key), std::move(value), OperationMode::FORCE_NO_LOCK);
      }
    }
  }
};