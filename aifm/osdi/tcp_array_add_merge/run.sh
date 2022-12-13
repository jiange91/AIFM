#!/bin/bash

log_folder=/mnt/AIFM/aifm/osdi/tcp_array_add_merge
cd $log_folder

source ../../shared.sh

cache_sizes=(1124)

sudo pkill -9 main

for cache_size in {100..1001..50}
do
    echo $cache_size
    cd ../../
    sed "s/constexpr static uint64_t kCacheMBs.*/constexpr static uint64_t kCacheMBs = $cache_size;/g" test/test_tcp_array_add_merge.cpp -i
    make 
    rerun_local_iokerneld_noht
    rerun_mem_server
    run_program_noht ./bin/test_tcp_array_add_merge 1>$log_folder/log.$cache_size 2>&1
    cd $log_folder
done
kill_local_iokerneld
