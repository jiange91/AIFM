#!/bin/bash

source ../shared.sh

# rerun_local_iokerneld_noht
# rerun_mem_server
# run_program_noht ../bin/interleave #1>log 2>&1
# kill_local_iokerneld

rerun_local_iokerneld_noht

rerun_mem_server
    
run_program_noht ../bin/interleave_merge

kill_local_iokerneld

kill_mem_server
