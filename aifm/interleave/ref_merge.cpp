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

using namespace far_memory;
using namespace std;

typedef long flow_t;
typedef long cost_t;

typedef struct node node_t;
typedef struct node *node_p;

typedef struct arc arc_t;
typedef struct arc *arc_p;

struct node
{
  cost_t potential; 
  int orientation;
  node_p child;
  node_p pred;
  node_p sibling;
  node_p sibling_prev;     
  arc_p basic_arc; 
  uint64_t firstout, firstin;
  arc_p arc_tmp;
  flow_t flow;
  long depth; 
  int number;
  int time;

  char padding[24];
};

struct arc
{
  cost_t cost;
  uint64_t tail, head;
  int ident;
  uint64_t nextout, nextin;
  flow_t flow;
  cost_t org_cost;
};

struct node_data_t {
  node_t data[32];
};

struct arc_data_t {
  arc_t data[64];
};

#define N_node (8 << 20)
#define M_arc (64 << 20)

static uint64_t seed = 0x23333;
static uint64_t checksum = 0xdeadbeaf; 


static inline int nextRand(int M) {

  seed = ((seed * 7621) + 1) % M;

  // static std::mt19937 g(seed);
  // static std::uniform_int_distribution<int> dist(0, M-1);
  // seed = dist(g);

  // printf("%d\n", (int)r);
  return (int)seed;
}

constexpr static uint64_t kCacheMBs = 5800;
constexpr static uint64_t kCacheSize = (kCacheMBs << 20);
constexpr static uint64_t kFarMemSize = (8ULL << 30);
constexpr static uint32_t kNumGCThreads = 40;
constexpr static uint32_t kNumConnections = 300;

void do_work(FarMemManager *manager) {
  /* Setup Starts. */ 
  // std::cout << "Setup" << std::endl;
  auto node_ = manager->allocate_array<node_data_t, N_node / 32>();
  auto arc_ = manager->allocate_array<arc_data_t, M_arc / 64>();
  
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < N_node / 32; ++i) {
    DerefScope scope;
    node_data_t &tmp = node_.at_mut(scope, i);
    for (int j = 0; j < 32; ++j) {
      tmp.data[j].number = -(i * 32 + j);
      tmp.data[j].firstin = nextRand(M_arc);
      tmp.data[j].firstout = nextRand(M_arc);
    }
  }

  for (int i = 0; i < M_arc / 64; ++i) {
    DerefScope scope;
    arc_data_t &tmp = arc_.at_mut(scope, i);
    for (int j = 0; j < 64; ++j) {
      tmp.data[j].tail = nextRand(N_node);
      tmp.data[j].head = nextRand(N_node);
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
  for(int i = 0; i < M_arc / 64; i++) {
    DerefScope scope;
    arc_data_t &tmp = arc_.at_mut(scope, i);
    for (int j = 0; j < 64; ++j) {
      uint64_t ti = tmp.data[j].tail;
      tmp.data[j].nextout = node_.at_mut(scope, ti >> 5).data[ti & 31].firstout;

      ti = tmp.data[j].tail;
      node_.at_mut(scope, ti >> 5).data[ti & 31].firstout = i * 64 + j;

      ti = tmp.data[j].head;
      tmp.data[j].nextin = node_.at_mut(scope, ti >> 5).data[ti & 31].firstin;

      ti = tmp.data[j].head;
      node_.at_mut(scope, ti >> 5).data[ti & 31].firstin = i * 64 + j;
    }
  }

  end = std::chrono::steady_clock::now();
  std::cout << "visit: "
    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    << " us" << std::endl;
  // /* Visit Ends. */


  // /* Check Starts */
  // std::cout << "Check" << std::endl; 
  start = std::chrono::steady_clock::now();
  uint64_t check_sum = 0;
  for (int i = 0; i < M_arc / 64; ++ i) {
    DerefScope scope;
    arc_data_t &tmp = arc_.at_mut(scope, i);
    for (int j = 0; j < 64; ++j) {
      uint64_t ti = tmp.data[j].tail;
      check_sum += node_.at_mut(scope, ti >> 5).data[ti & 31].number;
      
      ti = tmp.data[j].head;
      check_sum += node_.at_mut(scope, ti >> 5).data[ti & 31].number;
    }
  }
  printf("Checksum = %lu\n", check_sum);
  end = std::chrono::steady_clock::now();
  std::cout << "check: "
    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    << " us" << std::endl;
  /* Check End */
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

