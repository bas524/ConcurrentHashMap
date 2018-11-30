#include <iostream>
#include <thread>
#include "ConcurrentHashMap.h"

int main() {
  using TestMapType = ConcurrentHashMap<int, std::string>;

  TestMapType concurrentHashMap;

  for (int i = 0; i < 10000; ++i) {
    concurrentHashMap.insert(i, std::to_string(i));
  }

  //  concurrentHashMap.doForEeach(
  //      [](const TestMapType::ValueType &pair) {
  //        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
  //      });
  std::cout << " --------------- " << std::endl;

  std::thread thr1([&concurrentHashMap]() { concurrentHashMap.applyForEeach([](TestMapType::ValueType &pair) { pair.second.append("+1"); }); });

  std::thread thr2([&concurrentHashMap]() { concurrentHashMap.applyForEeach([](TestMapType::ValueType &pair) { pair.second.append("+2"); }); });

  thr1.join();
  thr2.join();

  concurrentHashMap.doForEeach([](const TestMapType::ValueType &pair) { std::cout << pair.first << " [" << pair.second << "]" << std::endl; });

  std::cout << " --------------- " << std::endl;

  std::thread thr3([&concurrentHashMap]() {
    concurrentHashMap.applyForEeachIf([](TestMapType::ValueType &pair) { pair.second.append("+even"); }, [](const TestMapType::ValueType &pair) { return pair.first % 2 == 0; });
  });

  std::thread thr4([&concurrentHashMap]() {
    concurrentHashMap.applyForEeachIf([](TestMapType::ValueType &pair) { pair.second.append("+not_even"); }, [](const TestMapType::ValueType &pair) { return pair.first % 2 != 0; });
  });

  thr3.join();
  thr4.join();

  concurrentHashMap.doForEeach([](const TestMapType::ValueType &pair) { std::cout << pair.first << " [" << pair.second << "]" << std::endl; });

  //  std::cout << " --------------- " << std::endl;
  //  concurrentHashMap.applyForEeachIf(
  //      [](TestMapType::ValueType &pair) {
  //        pair.second.append("-");
  //        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
  //      },
  //      [](TestMapType::ValueType &pair) {
  //        return pair.first % 3 == 0;
  //      });
  //
  //  std::cout << "size         : " << concurrentHashMap.size() << std::endl;
  //  std::cout << "empty      ? : " << (concurrentHashMap.empty() ? "true" : "false") << std::endl;
  //  std::cout << "contains 1 ? : " << (concurrentHashMap.contains(1) ? "true" : "false") << std::endl;
  //
  //  concurrentHashMap.rwLock().lock();
  //  concurrentHashMap.insert(11, std::to_string(11), TestMapType::OperationMode::FORCE_NO_LOCK);
  //  concurrentHashMap.applyForEeach(
  //      [](TestMapType::ValueType &pair) {
  //        pair.second.append("0");
  //        std::cout << pair.first << " [" << pair.second << "]" << std::endl;
  //      }, TestMapType::OperationMode::FORCE_NO_LOCK);
  //  concurrentHashMap.rwLock().unlock();
  return 0;
}