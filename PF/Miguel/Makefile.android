obj-m +=  fifomultimod.o
fifomultimod-objs += fifomulti.o cbuffer.o

ANDROIDSRC=/home/kernel/kitkat-src
KERNEL_TREE=$(ANDROIDSRC)/kernel
KERNEL_OUT=$(ANDROIDSRC)/out/target/product/x86/obj/kernel

all:
	make -C $(KERNEL_TREE) O=$(KERNEL_OUT) M=$(PWD) modules

clean:
	make -C $(KERNEL_TREE) O=$(KERNEL_OUT) M=$(PWD) clean

