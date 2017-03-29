#!/bin/bash

for (( ; ; ))
do
	echo "Copying /lib/x86_64-linux-gnu/libcrypto.so.1.0.0..."
 	cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1	
	echo "Checking copy of  /lib/x86_64-linux-gnu/libcrypto.so.1.0.0..."
	diff /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 fs1
done
