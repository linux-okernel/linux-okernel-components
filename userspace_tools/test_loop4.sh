#!/bin/bash
loops=$1
echo "Will loop for: $loops"

start_meminfo=$(/bin/cat /proc/meminfo |  awk '/MemFree/ {print $2}')

for i in `seq 1 $loops`; 
do
        cat /proc/meminfo | grep -i memfree
        taskset 0x1 /home/cid/projects/ckernel/okernel-usrc/okernel_test1	
done
end_meminfo=$(/bin/cat /proc/meminfo |  awk '/MemFree/ {print $2}')
echo start mem: $start_meminfo
echo end_mem  : $end_meminfo
used_mem=$(( $start_meminfo - $end_meminfo ))
echo Used Memory $used_mem
mem_perrun=$(( $used_mem / $loops ))
echo Used Memory per run $mem_perrun
