# Test programs to test linux-okernel protection/detection

When inserted this module will flip page table permissions and then
read and write kernel text and ro data. It then patches the linux
banner, so that /proc/version is changed. Finally it uses the linux
kernel provided text_poke function to replace unlink_at() with a "C3" sled
(x86_64 return instruction). This disables the "rm" instruction.

When you unload the module (highly recommended!) normality is restored.

To build:

`make clean && make`

`make clean -f makefile.bypass && make -f makefile.bpass`

To run:

`sudo insmod kwriter.ko`

Look in the linux okernel logs to see what linux-okernel is detecting,
if you run in okernel mode. The module is also pretty verbose, do
dmesg will show you what it is doing.

You can use the script chkresults to check it is all working as expected:
```
$ ./chkresults
Starting comparision beteen dmesg and okernel log
Success okernel detected changes to the following addresses set(['0xffffffff8ba00060', '0xffffffff8b391000', '0xffffffff8b400000', '0xffffffff8ba03000', '0xffffffff8b791000', '0xffffffff8b2882b0', '0xffffffff8ba04000', '0xffffffff8b200000', '0xffffffff8b000000', '0xffffffff8bdc0000', '0xffffffff8bdc1000', '0xffffffff8b591000', '0xffffffff8bdbf000', '0xffffffff8ba02000'])
```

Sample output from the command line shown below.

```
$ sudo insmod kwriter.ko
$ ./chkresults
Starting comparision beteen dmesg and okernel log
Success okernel detected changes to the following addresses set([-- deleted --])
$ cat /proc/version
Successfully patched linux_proc_banner
$ touch /tmp/test
$ rm /tmp/test
rm: cannot remove '/tmp/test'
$ sudo rm /tmp/test
rm: cannot remove '/tmp/test'
$ sudo rmmod kwriter
$ rm /tmp/test
$ cat /proc/version
Linux version 4.13.0+ (nje@cos-05) (gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)) #56 SMP Thu Sep 7 17:22:07 BST 2017
$
```
