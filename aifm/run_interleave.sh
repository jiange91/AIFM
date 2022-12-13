source shared.sh

rerun_local_iokerneld_noht

rerun_mem_server
    
run_program_noht ./bin/interleave

kill_local_iokerneld

kill_mem_server
