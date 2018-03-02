# linux-okernel building/running Instructions


## Building

1. Clone the main split kernel repo from https://github.com/linux-okernel/linux-okernel.

2. Checkout the 'linux-okernel' branch.

3. Build that kernel using the config file matching the desired kernel version from the *build_configs* sub-directory as the '.config' for the kernel.
   Note: If you make modifications to the config file or create your own, ensure that __'CONFIG_PAGE_TABLE_ISOLATION' is NOT set__,  __'CONFIG_VMAP_STACK' is NOT set__ and __'CONFIG_OKERNEL' IS set__ (i.e. CONFIG_OKERNEL=y).

4. Build the userspace tools found in sub-directory *userspace_tools* of this repository (linux-okernel-components).


## Running

To launch a shell in outer-kernel mode run (as root):

`$ ./userspace_tools/okernel_exec2 /bin/bash`

You can check its status by looking at /proc/self/okernel: it should be 1 when running in outer-kernel mode, 0 otherwise.


### Running containers in outer-kernel mode

To run containers in outer-kernel mode you currently need to start the docker daemon from a shell running in outer-kernel mode, e.g.:

`$ ./userspace_tools/okernel_exec2 /bin/bash`

`$ dockerd -D`


Any containers started via that daemon should be running in outer-kernel mode.

# Tests and demonstrations
Build and run the tests and demonstration of protected memory. See 'test_mappings/README' for more information.
