#include <iostream>
#include <memory>
#include <list>
#include <cassert>
#include <chrono>

struct Data {
  uint32_t data;
  // uint32_t dummy[1023];

  Data(uint32_t _data) : data(_data) {}
};

constexpr uint64_t kCacheSize = (1ULL << 30);
constexpr uint64_t kFarMemSize = (8ULL << 30);
constexpr uint32_t kNumGCThreads = 12;
constexpr uint32_t kNumDataEntries = 256 << 10;


int main() {
  std::list<Data> list;

  auto t1 = std::chrono::steady_clock::now();

  for (uint32_t i = 0; i < kNumDataEntries; i++) {
    list.push_back(Data(i));
  }

  uint32_t idx = 0;
  for (auto iter = list.begin(); iter != list.end(); iter++, idx++) {
    assert(iter.deref(scope).data == idx);
  }
  
  assert(idx == kNumDataEntries);
  auto t2 = std::chrono::steady_clock::now();

  std::cout << "copy: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
    << " us" << std::endl;
  
  std::cout << "Passed" << std::endl;

  return 0;
} 


