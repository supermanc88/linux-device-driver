obj-m := kern_rw_file.o

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNEL_SRC) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.symvers *.cmd *.cmd.o
