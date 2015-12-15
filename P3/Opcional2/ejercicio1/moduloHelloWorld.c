#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("GPL");

int hello_init(void){
	printk (KERN_INFO, "Hola.\n");
	return 0;
}

void cleanup_module (void){
	printk (KERN_INFO, "Adios.\n");
}
