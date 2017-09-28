#Instructions for demonstrating and testing Linux-okernel protected memory

## Building and loading the fake kernel vulnerability module

Adjust the KERNELDIR variable in 'test_mappings/kvmod/Makefile' (line
3) to reflect the path of the main split kernel repo (~/linux-okernel/
by default).

From the kvmod subdir:

`$ make clean && make`
`$ insmod ./kernel_vuln.ko`


## Building and running the test progs as stand alone binaries

**The following needs to be done from a shell in "okernel" mode unless
 stated otherwise**. See top level README.md for instructions to run a
 shell in outer-kernel or okernel mode.

From the pmc subdir:

`$ make clean && make`

From the kvc subdir:

`$ make clean && make`

Run in one shell:

`$ ./pmc/protected\_memory\_client`

And in another:

`$ ./kvc/kernel_vuln_client 0xAAAAAA (address from running the protected\_memory\_client prog)`

Repeat this in another shell that is **NOT in okernel mode**


## Building and running the test progs in a container

### protected\_memory\_client

From the pmc subdir:

`$ docker build -t pmc-app .`

`$ docker run --device=/dev/okernel:/dev/okernel -it --rm --name pmc pmc-app`

### kernel\_vuln\_client

From the kvc subdir:

`$ docker build -t kvc-app .`

`$ docker run --device=/dev/kernel\_vuln:/dev/kernel\_vuln -it --rm -e "KV_PHYSADDR=0xADDR" kvc-app`
