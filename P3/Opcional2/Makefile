obj-m +=  fifomod.o
fifomod-objs += charfifo.o cbuffer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
