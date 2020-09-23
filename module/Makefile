CONFIG_SPARSE_IRQ=y

obj-m := hw2.o

hw2-objs := mm_info_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

