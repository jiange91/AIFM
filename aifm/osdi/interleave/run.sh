#!/bin/bash

log_folder=/mnt/AIFM/aifm/osdi/interleave
cd $log_folder

source ../../shared.sh

cache_sizes=(1124)

sudo pkill -9 main

for cache_size in {8500..10001..500}
do
    echo $cache_size
    cd ../../
    sed "s/constexpr static uint64_t kCacheMBs.*/constexpr static uint64_t kCacheMBs = $cache_size;/g" interleave/ref.cpp -i
    make 
    rerun_local_iokerneld_noht
    rerun_mem_server
    run_program_noht ./bin/interleave 1>$log_folder/log.$cache_size 2>&1
    cd $log_folder
done
kill_local_iokerneld
