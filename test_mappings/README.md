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

Descriptions and instructions for running the exploit protection
demonstration and tests are given in the subdirectory README.md files:
* kwriter: general writing and patching of kernel code and data
* smepsmap: smep bypass using a publicly known technique. Adapted from
POC exploit for CVE-2017-7308
* vsys-update: write to the vsys memory page. Adapted from POC exploit
for CVE-2016-8655.

The POC exploits listed above are container in the exploits directory.

Instructions for demonstrating and tested the protected memory
facility of linux-okernel are given below.

# General Instructions for running protected memory test and demonstration
From one shell run 'protected\_memory\_client' (pmc) to setup and write to an (EPT) protected memory page.
From another shell run 'kernel\_vuln\_client' (kvc) passing it the physical address of the protected memory
page, obtained from pmc. This will try and used the faked-up kernel vulnerability (via kernel_vuln.ko) to try and
access the data in that page from kernel mode.


## Building and loading the fake kernel vulnerability module

From the kvmod subdir:

`$ make build`
`$ insmod ./kernel_vuln.ko`


## Building and running the test progs as stand alone binaries

From the pmc subdir:

`$ make clean; make`

From the kvc subdir:

`$ make clean; make`

Run in one shell:

`\# ./pmc/protected\_memory\_client`

And in another:

`\# ./kvc/kernel_vuln_client 0xAAAAAA (address from running the protected\_memory\_client prog)`


## Building and running the test progs in a container

### protected\_memory\_client

From the pmc subdir:

`$ docker build -t pmc-app .`

`$ docker run --device=/dev/okernel:/dev/okernel -it --rm --name pmc pmc-app`

### kernel\_vuln\_client

From the kvc subdir:

`$ docker build -t kvc-app .`

`$ docker run --device=/dev/kernel\_vuln:/dev/kernel\_vuln -it --rm -e "KV_PHYSADDR=0xADDR" kvc-app`
