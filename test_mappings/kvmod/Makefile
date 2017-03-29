ifeq ($(KERNELRELEASE),)  

KERNELDIR=~/linux-okernel
KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
PWD := $(shell pwd)  

.PHONY: build clean 

build:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules  

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c  *.symvers *.order
else  

$(info Building with KERNELRELEASE = ${KERNELRELEASE}) 
obj-m :=    kernel_vuln.o  

endif
