#!/bin/sh
make  -j2 LOCALVERSION= clean
make  -j2 LOCALVERSION= build
make test_kernel_vuln
