obj-m := get-signature-code.o

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/$(shell uname -r)/build
#KERNEL_SRC = /lib/modules/3.10.0-693.el7.x86_64/build
EXTRA_CFLAGS += -g -O0

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.symvers *.cmd *.cmd.o *.mod.c *.order
