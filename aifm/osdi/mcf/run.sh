#!/bin/bash

log_folder=/mnt/AIFM/aifm/osdi/mcf
cd $log_folder

source ../../shared.sh


sudo pkill -9 main


work_size=345

for ratio in {16..0..-2}
do
    cache_size=`expr $work_size \* $ratio / 10`
    echo $cache_size
    cd ../../429.mcf/aifm/
    sed "s/constexpr static uint64_t kCacheMBs.*/constexpr static uint64_t kCacheMBs = $cache_size;/g" mcf.cpp -i
    make 
    rerun_local_iokerneld_noht
    rerun_mem_server
    run_program_noht ./mcf 1>$log_folder/log.$cache_size 2>&1
    cd $log_folder
done
kill_local_iokerneld
