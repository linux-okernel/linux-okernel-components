# Test programs to test linux-okernel protection of vsys region

The vsys region is sometimes targeted as a way of bypassing SMEP/SMAP.
It is mapped into userspace processes. If an attacker can make it
writable, they can inject code or data into the kernel. This was used
in the Proof of Concept for CVE-2016-8655 published by rebel. This bug
allowed an attacker to invoke and arbitrary function in the
kernel. Rebel's POC used it twice.  Once to invoke set_memory_rw() to
set the vsys areas to RW. It then writes a new systemctl into the vys
area, before using the bug again to invoke register_sysctl_table() to
register the new sysctl. The new systemctl is then used to gain a root
shell. We are grateful to rebel for sharing their work.

Since we the above bug has been fixed, we simulate by inserting kernel
modules. Linux-okernel detects/prevents the write to the vsys area.

__**CAVEAT EMPTOR: THIS WORKS ON A DUAL PROCESSOR Z640. ON OTHER
MACHINES I'VE SEEN THE KERNEL HANG OR DEADLOCK.**__

## Building
For this test to work you will need to compile your kernel with the following options.


`CONFIG_LEGACY_VSYSCALL_NATIVE=y`

`# CONFIG_LEGACY_VSYSCALL_EMULATE is not set`

Once you have set the options as above, build your kernel in the normal way.

Then build this test code as follows:

`make clean && make`

`make clean -f makefile.vsys && make -f makefile.vsys`

To run, make sure you are in okernel mode:
```
$ okernel_exec2 /bin/bash
$ cat /proc/self/okernel
1
$ # we are in okernel mode
$
```
Then:

`sudo insmod vsysrw.ko`

`./vsysupdate`

`sudo insmod regsysctl.ko`

Note that vsysupdate takes a few seconds to run.

Look at the messages in the debug buffer (dmesg) and the linux-okernel
logs to see what is going on.

Note that regsysctl.ko doesn't register the sysctl, as this seems to cause
an unhandled page fault in new kernels. Because of this, I only use dummy
addresses for modprobe_path, and proc_dostring in vsysupdate. If we could
successfully register the sysctl, then the code would need to be
updated to use the correct addresses. You should see something like:
```
[ 5300.938518] reg_sys_ctl invoking oktargets 0xffffffffab0745c0
[ 5300.938526] reg_sys_ctl returned from oktargets
[ 5300.938530] reg_sys_ctl Attempting to read VSYSCALL page
[ 5300.938534] reg_sys_ctl Found at (VSYSCALL+0xf00): hack
[ 5300.938538] reg_sys_ctl done __init
```
Look in the okernel logs to see what okernel is detecting:
```
$ sudo tail -3 /sys/kernel/debug/tracing/trace
           <...>-5262  [003] ....  9043.217140: vmx_launch: [OK_SEC] [R, cpu(3),pid(5262)] : Physical address 0x3be5e04850 with EPT prot 0x5 alias or change for kernel protected code mapped at 0xffffffffff600850 being created for virtual address 0xffffffffff600850, new EPT prot 0x7, uid 1000, command vsysupdate
           <...>-5261  [006] ....  9043.223448: vmx_launch: [OK_SEC] [R, cpu(6),pid(5261)] : Physical address 0x3be5e04850 with EPT prot 0x5 alias or change for kernel protected code mapped at 0xffffffffff600850 being created for virtual address 0xffffffffff600850, new EPT prot 0x7, uid 1000, command vsysupdate
           <...>-5272  [011] ....  9062.577436: module_ept_violation: [OK_SEC] [R, cpu(11),pid(5272)] : New module code at pa 0x1f72f7c000 va 0xffffffffc00bd000
nje@cos-05:~/linux-okernel-components/test_mappings/vsys-update$

```

nigel.edwards@hpe.com