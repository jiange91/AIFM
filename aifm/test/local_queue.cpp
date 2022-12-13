#include <queue>
#include <cstdint>
#include <chrono>
#include <cassert>
#include <iostream>

struct Data {
  uint32_t data;
  uint32_t dummy[1023];

  Data(uint32_t _data) : data(_data) {}
};

constexpr uint32_t kNumDataEntries = 512 * 1024;

int main() {
    std::queue<Data> queue;

    auto start = std::chrono::steady_clock::now();
  
    for (uint32_t i = 0; i < kNumDataEntries; i++) {
      queue.push(Data(i));
    }

    for (uint32_t i = 0; i < kNumDataEntries; i++) {
      assert(queue.cfront().data == i);
      queue.pop();
    }
    
    auto end = std::chrono::steady_clock::now();
  
    std::cout << "Time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
      << " us" << std::endl;
  

    std::cout << "Passed" << std::endl;
    return 0;
  }
