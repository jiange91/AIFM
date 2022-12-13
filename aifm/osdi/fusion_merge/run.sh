#!/bin/bash

log_folder=/mnt/AIFM/aifm/osdi/fusion_merge
cd $log_folder

source ../../shared.sh

cache_sizes=(1124)

sudo pkill -9 main

for cache_size in {4400..301..800}
do
    echo $cache_size
    cd ../../fusion/aifm_merge
    sed "s/constexpr static uint64_t kCacheMBs.*/constexpr static uint64_t kCacheMBs = $cache_size;/g" ref.cpp -i
    make clean
    make
    rerun_local_iokerneld_noht
    rerun_mem_server
    run_program_noht ./fusion 1>$log_folder/log.$cache_size 2>&1
    cd $log_folder
done
kill_local_iokerneld
