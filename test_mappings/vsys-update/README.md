# Test programs to test linux-okernel protection of vsys region

The vsys region is sometimes targeted as a way of bypassing SMEP/SMAP.
It is mapped into userspace processes. If an attacker can make it writable,
they can inject code or data into the kernel. This was used in the Proof of
Concept for CVE-2016-8655 published by rebel. They used it to register
a new systemctl which they used in turn to gain a root shell. We are grateful
to rebel for sharing their work.

For this test to work you will need to compile your kernel with the following options.

CAVEAT EMPTOR: this works on a dual process Z640. On other machines I've seen the kernel hang or deadlock.

`CONFIG_LEGACY_VSYSCALL_NATIVE=y`

`# CONFIG_LEGACY_VSYSCALL_EMULATE is not set`

Then build using:

`make clean && make`

`make clean -f makefile.vsys && make -f makefile.vsys`

To run:

`sudo insmod vsysrw.ko`

`./vsysupdate`

`sudo insmod regsysctl.ko`

Look at the messages in the debug buffer (dmesg) and the linux-okernel
logs to see what is going on.

nigel.edwards@hpe.com