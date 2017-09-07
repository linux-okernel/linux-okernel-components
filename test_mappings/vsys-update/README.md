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

For this test to work you will need to compile your kernel with the following options.

CAVEAT EMPTOR: this works on a dual processor Z640. On other machines I've seen the kernel hang or deadlock.

`CONFIG_LEGACY_VSYSCALL_NATIVE=y`

`# CONFIG_LEGACY_VSYSCALL_EMULATE is not set`

Then build using:

`make clean && make`

`make clean -f makefile.vsys && make -f makefile.vsys`

To run:

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
updated to use the correct addresses.

nigel.edwards@hpe.com