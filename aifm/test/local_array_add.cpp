#include <cstdint>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <cstring>
#include <memory>
#include <random>
#include <string>

using namespace std;

constexpr static uint32_t kNumEntries =
    (8ULL << 20);

uint64_t raw_array_A[kNumEntries];
uint64_t raw_array_B[kNumEntries];
uint64_t raw_array_C[kNumEntries];

const int N = kNumEntries / 512;

struct obj_t {
  uint64_t data[512];
};

void copy_array(obj_t *array,  uint64_t *raw_array) {
  for (uint64_t i = 0; i < N; i++) {
    obj_t& tmp = array[i];
    for (int j = 0; j < 512; ++j) {
      tmp.data[j] = raw_array[i * 512 + j];
    }
  }
}

void add_array(obj_t *array_C, obj_t *array_A, obj_t *array_B) {
  for (uint64_t i = 0; i < N; i++) {
    const obj_t& tmpA = array_A[i];
    const obj_t& tmpB = array_B[i];
    obj_t& tmpC = array_C[i];
    for (int j = 0; j < 512; ++j) {
      tmpC.data[j] = tmpA.data[j] + tmpB.data[j];
    }
  }
}

void gen_random_array(uint64_t num_entries, uint64_t *raw_array) {
  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<uint64_t> distr;

  for (uint64_t i = 0; i < num_entries; i++) {
    raw_array[i] = distr(eng);
  }
}

int main() {
  obj_t *array_A = (obj_t*) malloc(kNumEntries << 3);
  obj_t *array_B = (obj_t*) malloc(kNumEntries << 3);
  obj_t *array_C = (obj_t*) malloc(kNumEntries << 3);

  gen_random_array(kNumEntries, raw_array_A);
  gen_random_array(kNumEntries, raw_array_B);
  
  auto t1 = std::chrono::steady_clock::now();
  copy_array(array_A, raw_array_A);
  auto t2 = std::chrono::steady_clock::now();
  copy_array(array_B, raw_array_B);
  auto t3 = std::chrono::steady_clock::now();
  add_array(array_C, array_A, array_B);
  auto t4 = std::chrono::steady_clock::now();
  
  std::cout << "copy: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()
    << " us" << std::endl;
  
  std::cout << "copy: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
    << " us" << std::endl;
  
  std::cout << "add: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count()
    << " us" << std::endl;

  for (uint64_t i = 0; i < N; i++) {
    for (int j = 0; j < 64; ++j) {
if (array_C[i].data[j] != raw_array_A[i*512+j] + raw_array_B[i*512+j]) {
      goto fail;
    }
    }
    
  }

  cout << "Passed" << endl;
  return 0;

fail:
  cout << "Failed" << endl;
  return 0;
}
