# ConcurrentHashMap

ConcurrentHashMap implementation with functional (lambda) signatures. For read-write locks I used std::shared_timed_mutex.
C++14 requred.

## Example

```c++
#include <iostream>
#include <thread>
#include "ConcurrentHashMap.h"

int main() {
  using TestMapType = ConcurrentHashMap<int, std::string>;

  TestMapType concurrentHashMap;

  for (int i = 0; i < 10000; ++i) {
    concurrentHashMap.insert(i, std::to_string(i));
  }
  
  std::cout << " --------------- " << std::endl;

  std::thread thr1([&concurrentHashMap]() { concurrentHashMap.doForEeach([](TestMapType::ValueType &pair) { pair.second.append("+1"); }); });

  std::thread thr2([&concurrentHashMap]() { concurrentHashMap.doForEeach([](TestMapType::ValueType &pair) { pair.second.append("+2"); }); });

  thr1.join();
  thr2.join();

  concurrentHashMap.doForEeach([](const TestMapType::ValueType &pair) { std::cout << pair.first << " [" << pair.second << "]" << std::endl; });

  std::cout << " --------------- " << std::endl;
}
```
