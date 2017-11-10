# Tests tracking of userspace memory
This intended for testing by okernel developers. It does not
demonstrate protection against exploits or other security features. If
you are curious read on....

Mode-based execute control, means that we can apply separate controls
to execution in ring 3 (user space) and ring < 3 (supervisor mode).

Unfortunately older processors do not have this, so when we set EPT_X
it grants execute permission regardless of in which ring the non root
mode execution is taking place. This means a page allocated to
userspace with EPT_X set needs to have the EPT_X bit cleared when it
is freed. Otherwise it could be reused in kernelspace with the EPT_X
set. It could then be used to execute new kernel code without being
detected which is something that we want to prevent.

## Building

`$ make clean && make`

## Running

If you have not already done so, you will need to insert the module
which initializes the oktest ioctl. See the instructions in
../oktest-init/README.md

You need to run some workloads in okernel mode, e.g. repeated building
of the kernels.

The run uxt in a loop. Something like:
`for ((;;)); do ./uxt && dmesg | grep oktest && sleep 30; done`


## If XPFO is enabled

If you have XPFO enabled, then you will get additional messages,
because any userspace allocated page that we need to mark EPT_X should
already be marked as userspace by XPFO. So we check that we agree with
XPFO. Note that it is not the case that all pages marked as userspace
by XPFO will be EPT_X. So we can just use XPFO to do our tracking. 

