#Kernel modules
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)


obj-m := sys_test.o


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONE:clean

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

rpi3b:
	$(MAKE) -C ../../../linux M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- clean
	$(MAKE) -C ../../../linux M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

