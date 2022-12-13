#include <vector>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <cstdint>

extern "C" {
#include <runtime/runtime.h>
}

#include "array.hpp"
#include "device.hpp"
#include "helpers.hpp"
#include "manager.hpp"

using namespace std;
using namespace far_memory;

#define ARY_SIZE (4ULL << 27)

typedef size_t size_type;
typedef size_t index_type;
typedef uint64_t dat_type;

struct data_t{
  uint64_t data[64];
};

typedef Array<data_t, ARY_SIZE / 64> array_t;


static inline uint64_t _microtime() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t t = ts.tv_sec*1000*1000 + ts.tv_nsec/1000;
    return t;
}

void ref_gen_uniform(size_t n_access, array_t &result, int seed = 2333) {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<> d(0, n_access - 1);
  for (size_t i = 0; i < n_access / 64; ++ i) {
    DerefScope scope;
    data_t& tmp = result.at_mut(scope, i);
    for (size_t j = 0; j < 64; ++j) {
      tmp.data[j] = d(gen);
    }
  }
}

void ref_gen_seq(size_t n_access, array_t &result)
{
  size_t p = 0;
  for (size_t i = 0; i < n_access / 64; ++i) {
    DerefScope scope;
    data_t& tmp = result.at_mut(scope, i);
    for (size_t j = 0; j < 64; ++j) {
      tmp.data[j] = p++;
    }
  }
}

class Visitor {
 public:
  uint64_t sum = 0;
  void pre() {}
  void post() {}
  void operator()(int a, int b) {sum += a; sum += b;}  
};

template <typename I, typename D>
class MaxVisitor {
public:
  I index_ = 0;
  D max_ = 0;
  bool is_first = true;

  void pre() {}
  void post() {}
  void operator()(I idx, D dat) {
    if (is_first || dat > max_) {
      max_ = dat;
      index_ = idx;
      is_first = false;
    }
  }
};

template <typename I, typename D>
class MinVisitor {
public:
  I index_ = 0;
  D min_ = 0;
  bool is_first = true;

  void pre() {}
  void post() {}
  void operator()(I idx, D dat) {
    if (is_first || dat < min_) {
      min_ = dat;
      index_ = idx;
      is_first = false;
    }
  }
};

template <typename I, typename D>
class MeanVisitor {
public:
  D mean_ = 0;
  size_type cnt_ = 0;

  void pre() { mean_ = 0; cnt_ = 0; }
  void post() {}
  void operator()(I idx, D dat) {
    mean_ += dat;
    cnt_ ++;
  }
  size_type get_count () const  { return (cnt_); }
  D get_sum () const  { return (mean_); }
  D get_result () const  {
    return (mean_ / D(cnt_));
  }
  
};

#define ARY_SIZE (4ULL << 27)
vector<dat_type> v;
vector<index_type> indices;

// void setup() {
//   v.reserve(ARY_SIZE);
//   indices.reserve(ARY_SIZE);

//   ref_gen_uniform(ARY_SIZE, v, 2333);
//   ref_gen_seq(ARY_SIZE, indices);
// }

extern void post_setup();

template<typename V1, typename V2, typename V3>
extern void visit (array_t& indices_, array_t& vec, V1 &visitor1, V2 &visitor2, V3 &visitor3);

void do_work (FarMemManager *manager) {
  printf("work\n");
  auto v = manager->allocate_array<data_t, ARY_SIZE/64>();
  auto indices = manager->allocate_array<data_t, ARY_SIZE/64>();
  
  printf("allocate\n");

  ref_gen_uniform(ARY_SIZE, v, 2333);
  printf("ref_gen_uniform\n");

  ref_gen_seq(ARY_SIZE, indices);
  printf("ref_gen_seq\n");

  MaxVisitor<index_type, dat_type> maxVst;
  MinVisitor<index_type, dat_type> minVst;
  MeanVisitor<index_type, dat_type> meanVst;

  uint64_t start = _microtime();
  visit(indices, v, maxVst, minVst, meanVst);
  uint64_t end = _microtime();

  printf("Time = %lu us\n", end-start);
  printf("Max vst = %lu %lu\n", maxVst.index_, maxVst.max_);
  printf("Min vst = %lu %lu\n", minVst.index_, minVst.min_);
  printf("Mean vst = %lu %lu\n", meanVst.get_count(), meanVst.get_result());
}
