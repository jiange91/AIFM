extern "C" {
#include <runtime/runtime.h>
}

#include <chrono>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdint>
#include <memory>
#include <random>
#include <string>

#include "array.hpp"
#include "device.hpp"
#include "helpers.hpp"
#include "manager.hpp"
#include "workload.hpp"

using namespace far_memory;
using namespace std;

const unsigned node_bb = 0;
const unsigned arc_bb = 6;
const uint64_t node_batch = (1 << node_bb);
const uint64_t arc_batch = (1 << arc_bb);

struct node_data_t {
  node_t data[node_batch];
};

struct arc_data_t {
  arc_t data[arc_batch];
};


constexpr static uint64_t kCacheMBs = 1000;
constexpr static uint64_t kCacheSize = (kCacheMBs << 20);
constexpr static uint64_t kFarMemSize = (8ULL << 30);
constexpr static uint32_t kNumGCThreads = 40;
constexpr static uint32_t kNumConnections = 300;

namespace far_memory{
class MissCalculator {
public:
  MissCalculator() {
    node_miss = 0;
    node_all = 0;
    arc_miss = 0;
    arc_all = 0;
  }

  void do_work(FarMemManager *manager) {
    /* Setup Starts. */ 
    // std::cout << "Setup" << std::endl;
    auto node_ = manager->allocate_array<node_data_t, N_node / node_batch>();
    auto arc_ = manager->allocate_array<arc_data_t, M_arc / arc_batch>();
    
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < N_node / node_batch; ++i) {
      // node_all += node_batch;
      DerefScope scope;
      node_data_t &tmp = node_.at_mut(scope, i);
      for (int j = 0; j < node_batch; ++j) {
        tmp.data[j].number = i * node_batch + j;
        tmp.data[j].firstin = dist2(g);
        tmp.data[j].firstout = dist2(g);
      }
    }

    for (uint64_t i = 0; i < N_node; ++ i) {
      node_list[i] = i;
    }
    std::shuffle(&node_list[0], node_list + N_node, g);

    for (int i = 0; i < M_arc / arc_batch; ++i) {
      // arc_all += arc_batch;
      DerefScope scope;
      arc_data_t &tmp = arc_.at_mut(scope, i);
      for (int j = 0; j < arc_batch; ++j) {
        tmp.data[j].tail = node_list[nextRand()];
        tmp.data[j].head = node_list[nextRand()];
      }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "setup: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
      << " us" << std::endl;
    /* Setup Ends. */

    /* Visit Starts. */
    // std::cout << "Visit" << std::endl;
    start = std::chrono::steady_clock::now();
    for(int i = 0; i < M_arc / arc_batch; i++) {
      // arc_all += 64;
      arc_miss += (!arc_.ptrs_[i].meta().is_present());
      
      DerefScope scope;
      arc_data_t &tmp = arc_.at_mut(scope, i);
      for (int j = 0; j < arc_batch; ++j) {
        uint64_t ti = tmp.data[j].head;
        // node_all += 1;
        node_miss += !node_.ptrs_[ti >> node_bb].meta().is_present();

        node_data_t &tmp_node = node_.at_mut(scope, ti >> node_bb);
        tmp.data[j].nextin = tmp_node.data[ti & node_batch].firstin;
        tmp_node.data[ti & node_batch].firstin = i * arc_batch + j;
        computation(tmp.data + (ti & node_batch), tmp.data[ti & node_batch].head)
      }
    }

    end = std::chrono::steady_clock::now();
    std::cout << "visit: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
      << " us" << std::endl;
    printf("node_miss: %.3f\n", 1.0 * node_miss / node_all);
    printf("arc_miss: %.3f\n", 1.0 * arc_miss / arc_all);
    // /* Visit Ends. */


    // /* Check Starts */
    // std::cout << "Check" << std::endl; 
    // start = std::chrono::steady_clock::now();
    // uint64_t check_sum = 0;
    // for (int i = 0; i < M_arc / 64; ++ i) {
    //   arc_all += 64;
    //   arc_miss += ((!arc_.ptrs_[i].meta().is_present()) << 6);
      
    //   DerefScope scope;
    //   arc_data_t &tmp = arc_.at_mut(scope, i);
    //   for (int j = 0; j < 64; ++j) {
    //     uint64_t ti = tmp.data[j].tail;      
    //     node_all += 1;
    //     node_miss += !node_.ptrs_[ti >> 5].meta().is_present();

    //     node_data_t &node_tmp = node_.at_mut(scope, ti >> 5);
    //     check_sum += node_tmp.data[ti & 31].number;
        
    //     ti = tmp.data[j].head;
    //     node_all += 1;
    //     node_miss += !node_.ptrs_[ti >> 5].meta().is_present();

    //     node_tmp = node_.at_mut(scope, ti >> 5);
    //     check_sum += node_tmp.data[ti & 31].number;
    //   }
    // }
    // printf("Checksum = %lu\n", check_sum);
    // end = std::chrono::steady_clock::now();
    // std::cout << "check: "
    //   << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    //   << " us" << std::endl;
    // /* Check End */
  }

private:
  uint64_t node_miss, node_all;
  uint64_t arc_miss, arc_all;
};
}

MissCalculator calculator;

int argc;
void _main(void *arg) {
  char **argv = static_cast<char **>(arg);
  std::string ip_addr_port(argv[1]);
  auto raddr = helpers::str_to_netaddr(ip_addr_port);
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads,
          new TCPDevice(raddr, kNumConnections, kFarMemSize)));
  calculator.do_work(manager.get());
}


int main(int _argc, char *argv[]) {
  std::cout << "sizeof: " << sizeof(arc_t) << ' ' << sizeof(node_t) << std::endl;
  
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

