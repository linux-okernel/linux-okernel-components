#!/bin/bash

for (( ; ; ))
do
	echo "Copying /lib/x86_64-linux-gnu/libcrypto.so.1.0.0..."
 	#/home/cid/projects/ckernel/okernel-usrc/okernel_exec2 /bin/cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1	
 	/bin/cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1	
 	/home/cid/projects/ckernel/okernel-usrc/okernel_exec2 /bin/cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1	
	echo "Checking copy of  /lib/x86_64-linux-gnu/libcrypto.so.1.0.0..."
	#/home/cid/projects/ckernel/okernel-usrc/okernel_exec2 /usr/bin/diff /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1
	/usr/bin/diff /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1
done
