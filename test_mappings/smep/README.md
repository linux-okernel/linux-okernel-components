# Test programs to test linux-okernel protection of SMEP

Kernel bugs allowing function pointers to be overwritten are somtimes
used to target native_cr4_write to bypase SMEP/SMAP. This is
demonstrated by Andrey Konovalov in his proof-of-concept local root
exploit for CVE-2017-7308. This is the inspiration behind this test
case, we are grateful to Andrey for publishing his work.

Since the latest kernels patched, we need to simulate the bug and exploit.
We use the IOCTL expose by oktest to simulate the effect of kernel
function pointer control. The ioctl writes to CR4 turning off SMEP and
SMAP and then executes the user supplied function, which because it is
in a userspace address would be blocked by SMEP and SMAP. The module
ok_test_init is used to enable/disable this ioctl in oktest.

## To build:

`make clean && make`

## To run:

Run bypass as a normal user (i.e. non-root) to get a root shell
simulating the exploit

`./bypass <address of commit_creds> <address of prepare_kernel_cred>`

or

`./bypass $(sudo ./params.sh)`

The script params.sh (needs to be run as root) can get the necessary
addresses, or you can get them manually from /proc/kallsyms


Look at the messages in the debug buffer (dmesg) and the linux-okernel
logs to see what is going on.

An example of the output in normal mode is shown below:
```
$ cat /proc/self/okernel
0
$ # We are not in okernel mode
$ id
uid=1000(nje) gid=1000(nje) groups=1000(nje),4(adm),24(cdrom),27(sudo),30(dip),46(plugdev),110(lxd),115(lpadmin),116(sambashare),129(docker)
$ ./bypass $(sudo ./params.sh)
Count is 0
Calling OKTEST_EXEC to bypass SMEP with func1
Count is 1
Calling OKTEST_EXEC to bypass SMEP with get_root_payload
OKTEST_EXEC done
[.] checking if we got root
[+] got r00t ^_^
$ id
uid=0(root) gid=0(root) groups=0(root)
$
```

Exit the root shell, and start a shell in okernel mode (see top level
README file). Repeat the run of bypass in okernel mode. In okernel
mode "bypass" is killed on the first SMEP bypass attempt after
printing it is calling func1. You can check that okernel is detecting
this by looking in the log file as shown below.

```
$ okernel_exec2 /bin/bash
$ cat /proc/self/okernel
1
$ # We are in okernel mode
$ ./bypass $(sudo ./params.sh)
Count is 0
Calling OKTEST_EXEC to bypass SMEP with func1
$ sudo tail -1 /sys/kernel/debug/tracing/trace
           <...>-4685  [008] ....  8367.519116: vmx_launch: [OK_SEC] [R, cpu(8),pid(4685)] : okernel blocking write to CR4: SMEP exploit attempt? (uid 1000, pid 4685, command bypass, access type 0, register 6, reg value 0x606e0)
$
```

nigel.edwards@hpe.com
