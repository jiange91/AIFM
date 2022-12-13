#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <random>
#include <iostream>
#include <cstdio>
#include <chrono>

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
  arc_p firstout, firstin;
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
  node_p tail, head;
  int ident;
  arc_p nextout, nextin;
  flow_t flow;
  cost_t org_cost;
};

int a = sizeof(arc_t);
int b = sizeof(node_t);

node_t *node;
arc_t *arc;

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

extern void setup();
extern void visit();
extern void check();

void do_work() {
  auto start = std::chrono::steady_clock::now();
  setup();
  auto end = std::chrono::steady_clock::now();
  std::cout << "setup: "
    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    << " us" << std::endl;
  
  start = std::chrono::steady_clock::now();
  visit();
  end = std::chrono::steady_clock::now();
  std::cout << "visit: "
    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    << " us" << std::endl;
  
  start = std::chrono::steady_clock::now();
  check();
  end = std::chrono::steady_clock::now();
  std::cout << "check: "
    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    << " us" << std::endl;
}

void setup() {
  node = (node_t *) malloc(sizeof(node_t) * N_node);
  arc = (arc_t *) malloc(sizeof(arc_t) * M_arc);

  for (int i = 0; i < N_node; ++ i) {
    node[i].number = -i;
    node[i].firstin = arc + nextRand(M_arc);
    node[i].firstout = arc + nextRand(M_arc);
  }

  for (int i = 0; i < M_arc; ++ i) {
    arc[i].tail = node + nextRand(N_node);
    arc[i].head = node + nextRand(N_node);
  }
}

void visit() {
  for( int i = 0; i < M_arc; i++ )
  {
    arc[i].nextout = arc[i].tail->firstout;
    arc[i].tail->firstout = arc + i;
    arc[i].nextin = arc[i].head->firstin;
    arc[i].head->firstin = arc + i;
  }
}

void check() {
  uint64_t check_sum = 0;
  for (int i = 0; i < M_arc; ++ i) {
    check_sum += arc[i].tail->number;
    check_sum += arc[i].head->number;
  }
  printf("Checksum = %lu\n", check_sum);
}

int main () {

  do_work();
  return 0;
}