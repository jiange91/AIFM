#include <vector>
#include <cstdio>
#include <cstdlib>
#include "workload.h"

void post_setup() { return; }

template<typename V1, typename V2, typename V3>
void visit (array_t& indices_, array_t& vec, V1 &visitor1, V2 &visitor2, V3 &visitor3) {

  // const size_type idx_s = indices_.size();
  // const size_type min_s = std::min<size_type>(vec.size(), idx_s);
  const size_type min_s = ARY_SIZE;
  size_type       i = 0;

  visitor1.pre();
  for (; i < min_s; ++i) {
    DerefScope scope;
    visitor1 (indices_.at(scope, i), vec.at(scope, i));
  }
  visitor1.post();

  visitor2.pre();
  for (i = 0; i < min_s; ++i) {
    DerefScope scope;
    visitor2 (indices_.at(scope, i), vec.at(scope, i));
  }
  visitor2.post();

  visitor3.pre();
  for (i = 0; i < min_s; ++i) {
    DerefScope scope;
    visitor3 (indices_.at(scope, i), vec.at(scope, i));
  }
  visitor3.post();

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
  printf("main\n");
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

