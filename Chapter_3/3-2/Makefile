#Kernel modules
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)


obj-m := parametermodule.o calculation.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONE:clean

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean



