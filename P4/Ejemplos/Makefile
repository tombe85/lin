obj-m +=  example_timer.o  workqueue1.o workqueue2.o workqueue3.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
