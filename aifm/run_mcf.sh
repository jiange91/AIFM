source shared.sh

rerun_local_iokerneld_noht

rerun_mem_server
    
run_program_noht ./429.mcf/aifm/mcf

kill_local_iokerneld

kill_mem_server
