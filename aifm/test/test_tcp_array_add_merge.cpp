extern "C" {
#include <runtime/runtime.h>
}

#include "array.hpp"
#include "device.hpp"
#include "helpers.hpp"
#include "manager.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using namespace far_memory;
using namespace std;

constexpr static uint64_t kCacheMBs = 240;
constexpr static uint64_t kCacheSize = (kCacheMBs << 20);
constexpr static uint64_t kFarMemSize = (240 << 20); // (4ULL << 30);
constexpr static uint32_t kNumGCThreads = 12;
constexpr static uint32_t kNumEntries =
    (8ULL << 20); // So the array size is larger than the local cache size.
constexpr static uint32_t kNumConnections = 300;

struct obj_t {
  uint64_t data[512];
};

uint64_t raw_array_A[kNumEntries];
uint64_t raw_array_B[kNumEntries];
uint64_t raw_array_C[kNumEntries];

template <uint64_t N, typename T>
void copy_array(Array<T, N> *array, uint64_t *raw_array) {
  for (uint64_t i = 0; i < N; i++) {
    DerefScope scope;
    obj_t& tmpA = (*array).at_mut(scope, i);
    for (uint32_t j = 0; j < 512; ++j) {
      tmpA.data[j] = raw_array[i * 512 + j];
    }
  }
}

template <typename T, uint64_t N>
void add_array(Array<T, N> *array_C, Array<T, N> *array_A,
               Array<T, N> *array_B) {
  for (uint64_t i = 0; i < N; i++) {
    DerefScope scope;
    auto tmpA = (*array_A).at(scope, i);
    auto tmpB = (*array_B).at(scope, i);
    obj_t& tmpC = (*array_C).at_mut(scope, i);
    for (uint32_t j = 0; j < 512; ++j) {
      tmpC.data[j] =
        tmpA.data[j] + tmpB.data[j];
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

void do_work(FarMemManager *manager) {
  auto array_A = manager->allocate_array<obj_t, kNumEntries / 512>();
  auto array_B = manager->allocate_array<obj_t, kNumEntries / 512>();
  auto array_C = manager->allocate_array<obj_t, kNumEntries / 512>();

  gen_random_array(kNumEntries, raw_array_A);
  gen_random_array(kNumEntries, raw_array_B);
  auto t1 = std::chrono::steady_clock::now();
  copy_array(&array_A, raw_array_A);
  auto t2 = std::chrono::steady_clock::now();
  copy_array(&array_B, raw_array_B);
  auto t3 = std::chrono::steady_clock::now();
  add_array(&array_C, &array_A, &array_B);
  auto t4 = std::chrono::steady_clock::now();
  
  std::cout << "copy: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
    << " us" << std::endl;
  std::cout << "copy: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()
    << " us" << std::endl;
  std::cout << "add: "
    << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count()
    << " us" << std::endl;

  for (uint64_t i = 0; i < kNumEntries / 512; i++) {
    DerefScope scope;
    for (uint32_t j = 0; j < 512; ++j) {
      if (array_C.at(scope, i).data[j] != raw_array_A[i * 512 + j] + raw_array_B[i * 512 + j]) {
        goto fail;
      }
    }
  }

  cout << "Passed" << endl;
  return;

fail:
  cout << "Failed" << endl;
}

int argc;
void _main(void *arg) {
  char **argv = static_cast<char **>(arg);
  std::string ip_addr_port(argv[1]);
  auto raddr = helpers::str_to_netaddr(ip_addr_port);
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads,
          new TCPDevice(raddr, kNumConnections, kFarMemSize)));
  do_work(manager.get());
}

int main(int _argc, char *argv[]) {
  int ret;

  if (_argc < 3) {
    std::cerr << "usage: [cfg_file] [ip_addr:port]" << std::endl;
    return -EINVAL;
  }

  char conf_path[strlen(argv[1]) + 1];
  strcpy(conf_path, argv[1]);
  for (int i = 2; i < _argc; i++) {
    argv[i - 1] = argv[i];
  }
  argc = _argc - 1;

  ret = runtime_init(conf_path, _main, argv);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  return 0;
}
