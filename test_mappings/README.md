# Test cases and demonstration code

This directory contains demonstration and test code to show how
linux-okernel protects agains kernel exploits and provides protected
memory regions.

When running these tests in okernel mode, to look at the linux okernel
logs and see what it is detecting/blocking, do the following as root:
```
$ cat /sys/kernel/debug/tracing/trace > /tmp/trace.txt
$ cat /tmp/trace.txt
```
Note that since the bugs which the exploits used have been fixed, we
insert kernel modules to replicate the effect of these bugs.

Descriptions and instructions for running the exploit protection
demonstration and tests are given in the subdirectory README.md files:
* kwriter: general writing and patching of kernel code and data
* protected-mem: demonstration of protected memory
* smep: smep bypass using a publicly known technique. Adapted from
POC exploit for CVE-2017-7308
* user-mem-track: Tests tracking of userspace memory for older
processors that do not support mode-based execute control for EPT
* vsys-update: write to the vsys memory page. Adapted from POC exploit
for CVE-2016-8655.

In addition, not intended for testing, just for archival purposes and study:
* exploits: The POC exploits listed above and the motivation for our tests
