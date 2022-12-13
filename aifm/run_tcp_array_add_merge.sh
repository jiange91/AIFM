source shared.sh

rerun_local_iokerneld_noht

rerun_mem_server
    
run_program ./bin/test_tcp_array_add_merge


kill_local_iokerneld

kill_mem_server
