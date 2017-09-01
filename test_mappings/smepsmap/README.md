# Test programs to test linux-okernel protection of SMEP/SMAP

Kernel bugs allowing function pointers to be overwritten are somtimes
used to target native_cr4_write to bypase SMEP/SMAP. This is
demonstrated by Andrey Konovalov in his proof-of-concept local root
exploit for CVE-2017-7308. This is the inspiration behind this test
case, we are grateful to Andrey for publishing his work.

Since the latest kernels patched, we need to simulate the bug and exploit.

We use the IOCTL expose by oktest to simulate the effect of kernel
function pointer control. The ioctl writes to CR4 turning off SMEP and
SMAP and then executes the user supplied function, which because it is
in a userspace address would be blocked by SMEP and SMAP.

To build:

`make clean && make`

`make clean -f makefile.bypass && make -f makefile.bpass`

To run:

`sudo insmod cr4writer.ko`

All this kernel module does is initialize the ioctl device

`./bypass <address of commit_creds> <address of prepare_kernel_cred>`

The script params.sh (needs to be run as root) can get the necessary
addresses, or you can get them manually from /proc/kallsyms


Look at the messages in the debug buffer (dmesg) and the linux-okernel
logs to see what is going on.

An example of the output in normal mode is shown below:
```
nje@cos-04:~/linux-okernel-components/test_mappings/smepsmap$ id
uid=1000(nje) gid=1000(nje) groups=1000(nje),4(adm),24(cdrom),27(sudo),30(dip),46(plugdev),110(lxd),115(lpadmin),116(sambashare),129(docker)
nje@cos-04:~/linux-okernel-components/test_mappings/smepsmap$ ./bypass $(sudo ./params.sh)
Count is 0
Calling OKTEST_EXEC to bypass SMEP/SMAP with func1
Count is 1
Calling OKTEST_EXEC to bypass SMEP/SMAP with get_root_payload
OKTEST_EXEC done
[.] checking if we got root
[+] got r00t ^_^
root@cos-04:/home/nje/linux-okernel-components/test_mappings/smepsmap# id
uid=0(root) gid=0(root) groups=0(root)
root@cos-04:/home/nje/linux-okernel-components/test_mappings/smepsmap# 
```
nigel.edwards@hpe.com
