# ContainerOS Split Kernel Building / Running Instructions


## Building

1. Clone the main split kernel repo from https://github.com/linux-okernel/linux-okernel.

2. Checkout the 'linux-okernel' branch.

3. Build that kernel using the 'okernel.config' file from the *build_configs* sub-directory as the '.config' for the kernel.
   Note: If you make modifications to the config file or create your own, ensure that __'CONFIG_VMAP_STACK' is NOT set__ and __'CONFIG_OKERNEL' IS set__ (i.e. CONFIG_OKERNEL=y).

4. Build the userspace tools found in sub-directory *userspace_tools*.

5. Build the protected memory interface test programs and kernel module found in sub-directory *test_mappings*:
  - Adjust the KERNELDIR variable in 'test_mappings/kvmod/Makefile' (line 3) to reflect the path of the main split kernel repo (~/linux-okernel/ by default).
  - Build and load the fake kernel vulnerability module:

       From the *kvmod* sub-directory:
       
      `$ make build`
         
      `$ insmod ./kernel_vuln.ko`

  - Build and run the test programs as stand alone binaries:

       From the *pmc* sub-directory:
       `$ make clean; make`
   
       From the *kvc* sub-directory:
      `$ make clean; make`
      
      Run in one shell:
      `$ ./pmc/protected_memory_client`
      
      And in another:
      `$ ./kvc/kernel_vuln_client 0xAAAAAA (address from running the protected_memory_client prog)`

See 'test_mappings/README' for more information.


## Running

To launch a shell in outer-kernel mode run:

`$ ./userspace_tools/okernel_exec2 /bin/bash`

You can check its status by looking at /proc/self/okernel: it should be 1 when running in outer-kernel mode, 0 otherwise.


### Running containers in outer-kernel mode

To run containers in outer-kernel mode you currently need to start the docker daemon from a shell running in outer-kernel mode, e.g.:

`$ ./userspace_tools/okernel_exec2 /bin/bash`

`$ docker daemon -D`


Any containers then started via that daemon should be running in outer-kernel mode.
