#!/bin/bash

log_folder=/mnt/AIFM/aifm/osdi/interleave_merge_miss
cd $log_folder

source ../../shared.sh

cache_sizes=(1124)

sudo pkill -9 main

for cache_size in {3000..500..1000}
do
    echo $cache_size
    cd ../../
    sed "s/constexpr static uint64_t kCacheMBs.*/constexpr static uint64_t kCacheMBs = $cache_size;/g" interleave_miss/ref_merge.cpp -i
    make clean
    make all
    rerun_local_iokerneld_noht
    rerun_mem_server
    run_program_noht ./interleave_miss/main 1>$log_folder/log.$cache_size 2>&1
    cd $log_folder
done
kill_local_iokerneld
