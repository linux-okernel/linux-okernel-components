#!/bin/bash
grep ' prepare_kernel_cred\| commit_cred' /proc/kallsyms | \
    awk '{print($1)}' | \
    sed ':a;/0$/{N;s/\n/ /;ba}'
