obj-m+=input.o
obj-m+=output.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
load:
	insmod ./input.ko
	insmod ./output.ko
unload:
	rmmod ./output.ko
	rmmod ./input.ko
view:
	cat /proc/modules

