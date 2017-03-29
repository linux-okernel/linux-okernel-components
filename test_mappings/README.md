# General Instructions
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
