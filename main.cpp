#include <iostream>

#include "ConcurrentHashMap.h"

int main() {

  using TestMapType = ConcurrentHashMap<int, std::string>;

  TestMapType concurrentHashMap;

  for (int i = 0; i < 10; ++i) {
    concurrentHashMap.insert(i, std::to_string(i));
  }

  concurrentHashMap.doForEeach(
      [](const TestMapType::ValueType &pair) {
        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
      });
  std::cout << " --------------- " << std::endl;
  concurrentHashMap.applyForEeach(
      [](TestMapType::ValueType &pair) {
        pair.second.append("0");
        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
      });
  std::cout << " --------------- " << std::endl;
  concurrentHashMap.doForEeachIf(
      [](const TestMapType::ValueType &pair) {
        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
      },
      [](const TestMapType::ValueType &pair) {
        return pair.first % 2 == 0;
      });

  std::cout << " --------------- " << std::endl;
  concurrentHashMap.applyForEeachIf(
      [](TestMapType::ValueType &pair) {
        pair.second.append("-");
        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
      },
      [](TestMapType::ValueType &pair) {
        return pair.first % 3 == 0;
      });

  std::cout << "size         : " << concurrentHashMap.size() << std::endl;
  std::cout << "empty      ? : " << (concurrentHashMap.empty() ? "true" : "false") << std::endl;
  std::cout << "contains 1 ? : " << (concurrentHashMap.contains(1) ? "true" : "false") << std::endl;

  concurrentHashMap.rwLock().writeLock();
  concurrentHashMap.insert(11, std::to_string(11), TestMapType::OperationMode::FORCE_NO_LOCK);
  concurrentHashMap.applyForEeach(
      [](TestMapType::ValueType &pair) {
        pair.second.append("0");
        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
      }, TestMapType::OperationMode::FORCE_NO_LOCK);
  concurrentHashMap.rwLock().unlock();
  return 0;
}