# Module to initialize the oktest ioctl device

This module initialize a test ioctl. We don't want it available by
default. Hence we insert a module to turn it on and remove the module
to turn it off.

## To build:

`make clean && make`

## To run:

`sudo insmod oktest_init.ko`

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
sudo rmmod oktest_init.ko
sudo insmod oktest_init.ko
```

nigel.edwards@hpe.com
