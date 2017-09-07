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
dmesg will show you what it is doing. Sample output from the command
line shown below.

```
$ cat /proc/version
Linux version 4.13.0+ (nje@cos-05) (gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)) #56 SMP Thu Sep 7 17:22:07 BST 2017
$ sudo insmod kwriter.ko
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



