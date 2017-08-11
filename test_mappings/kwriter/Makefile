ifeq ($(KERNELRELEASE),)

KERNELDIR=~/linux-okernel
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: build clean

build:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules

install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) \
		INSTALL_MOD_PATH=$(INSTALL_ROOT) modules_install
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c  *.symvers *.order
else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    kwriter.o

endif
