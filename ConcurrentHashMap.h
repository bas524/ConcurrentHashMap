#include <Poco/RWLock.h>
#include <algorithm>
#include <unordered_map>

#define DEFENCE_SOCPE(x, lockMode)            \
  do {                                        \
    if (mode == OperationMode::WITH_LOCK) {   \
      _rwLock.lockMode();                     \
    }                                         \
    try {                                     \
      x;                                      \
    } catch (...) {                           \
      if (mode == OperationMode::WITH_LOCK) { \
        _rwLock.unlock();                     \
      }                                       \
      throw;                                  \
    }                                         \
    if (mode == OperationMode::WITH_LOCK) {   \
      _rwLock.unlock();                       \
    }                                         \
  } while (0)

#define DEFENCE_SOCPE_WITH_RESULT(res, x, lockMode) \
  do {                                              \
    if (mode == OperationMode::WITH_LOCK) {         \
      _rwLock.lockMode();                           \
    }                                               \
    try {                                           \
      res = x;                                      \
    } catch (...) {                                 \
      if (mode == OperationMode::WITH_LOCK) {       \
        _rwLock.unlock();                           \
      }                                             \
      throw;                                        \
    }                                               \
    if (mode == OperationMode::WITH_LOCK) {         \
      _rwLock.unlock();                             \
    }                                               \
  } while (0)

template <typename Key, typename Value>
class ConcurrentHashMap {
  std::unordered_map<Key, Value> _map;
  mutable Poco::RWLock _rwLock;

 public:
  enum class OperationMode { WITH_LOCK, FORCE_NO_LOCK };
  using ValueType = typename std::unordered_map<Key, Value>::value_type;
  ConcurrentHashMap() = default;
  explicit ConcurrentHashMap(size_t n) : _map(n) {}
  ConcurrentHashMap(std::initializer_list<ValueType> list, size_t n = 0) : _map(list, n) {}
  ConcurrentHashMap(const ConcurrentHashMap &o) {
    Poco::ScopedReadRWLock readRWLock(o._rwLock);
    _map = o._map;
  }
  ConcurrentHashMap(ConcurrentHashMap &&o) noexcept : _rwLock(o._rwLock) {
    Poco::ScopedWriteRWLock writeRWLock(o._rwLock);
    _map = std::move(o._map);
  }
  const Poco::RWLock &rwLock() const { return _rwLock; }
  Poco::RWLock &rwLock() { return _rwLock; }
  void insert(const Key &key, const Value &value, OperationMode mode = OperationMode::WITH_LOCK) { DEFENCE_SOCPE(_map.insert(std::make_pair(key, value)), writeLock); }
  void emplace(Key &&key, Value &&value, OperationMode mode = OperationMode::WITH_LOCK) { DEFENCE_SOCPE(_map.emplace(std::forward<Key>(key), std::forward<Value>(value)), writeLock); }
  bool empty(OperationMode mode = OperationMode::WITH_LOCK) const {
    bool result;
    DEFENCE_SOCPE_WITH_RESULT(result, _map.empty(), readLock);
    return result;
  }
  size_t size(OperationMode mode = OperationMode::WITH_LOCK) const {
    size_t result;
    DEFENCE_SOCPE_WITH_RESULT(result, _map.size(), readLock);
    return result;
  }
  bool contains(const Key &k, OperationMode mode = OperationMode::WITH_LOCK) const {
    bool result;
    DEFENCE_SOCPE_WITH_RESULT(result, _map.find(k) != _map.end(), readLock);
    return result;
  }
  template <typename Function>
  void doForEeach(Function function, OperationMode mode = OperationMode::WITH_LOCK) const {
    DEFENCE_SOCPE(std::for_each(_map.begin(), _map.end(), function), readLock);
  }
  template <typename Function>
  void applyForEeach(Function function, OperationMode mode = OperationMode::WITH_LOCK) {
    DEFENCE_SOCPE(std::for_each(_map.begin(), _map.end(), function), writeLock);
  }
  template <typename Function, typename Predicate>
  void doForEeachIf(Function function, Predicate predicate, OperationMode mode = OperationMode::WITH_LOCK) const {
    DEFENCE_SOCPE(
        for (const auto &item
             : _map) {
          if (predicate(item)) {
            function(item);
          }
        },
        readLock);
  }
  template <typename Function, typename Predicate>
  void applyForEeachIf(Function function, Predicate predicate, OperationMode mode = OperationMode::WITH_LOCK) {
    DEFENCE_SOCPE(
        for (auto &item
             : _map) {
          if (predicate(item)) {
            function(item);
          }
        },
        writeLock);
  }
};