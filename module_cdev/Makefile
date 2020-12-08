obj-m := module_cdev.o

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS=-g

all:
        $(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules
clean:
        rm -f *.ko *.o *.symvers *.cmd *.cmd.o *.mod.c *.order
