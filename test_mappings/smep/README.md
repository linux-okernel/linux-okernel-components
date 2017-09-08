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
in a userspace address would be blocked by SMEP and SMAP.

To build:

`make clean && make`

`make clean -f makefile.bypass && make -f makefile.bpass`

To run:

`sudo insmod cr4writer.ko`

All this kernel module does is initialize the ioctl device

For the IOCTL to be accessible to users other than root, you need
to make it non-root accessible. Do the following as root:
```
$ udevadm info -a -p /sys/class/oktest/oktest
```
The output will look like:
```
Udevadm info starts with the device specified by the devpath and then
walks up the chain of parent devices. It prints for every device
found, all possible attributes in the udev rules key format.
A rule to match, can be composed by the attributes of the device
and the attributes from one single parent device.

  looking at device '/devices/virtual/oktest/oktest':
    KERNEL=="oktest"
    SUBSYSTEM=="oktest"
    DRIVER==""
```
Create the file /etc/udev/rules.d/99-oktest.rules or similar as shown below
```
$ cd /etc/udev/rules.d/
$ vi 99-oktest.rules
$ cat 99-oktest.rules 
# Rule for oktest
KERNEL=="oktest", SUBSYSTEM=="oktest", MODE="0666"

$ 
```

Remove and reinsert the module so it is available to non-root users:
```
sudo rmmod cr4writer.ko
sudo insmod cr4writer.ko
```

Now run bypass as a normal user (i.e. non-root) to get a root shell
simulating the exploit

`./bypass <address of commit_creds> <address of prepare_kernel_cred>`

The script params.sh (needs to be run as root) can get the necessary
addresses, or you can get them manually from /proc/kallsyms


Look at the messages in the debug buffer (dmesg) and the linux-okernel
logs to see what is going on.

An example of the output in normal mode is shown below:
```
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

In okernel mode "bypass" is killed on the first SMEP bypass attempt
after printing

`Calling OKTEST_EXEC to bypass SMEP with func1`

nigel.edwards@hpe.com
