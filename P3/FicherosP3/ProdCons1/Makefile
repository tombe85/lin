obj-m +=  pcmod1.o 
pcmod1-objs += cbuffer.o prodcons1.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

