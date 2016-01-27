#include <linux/kfifo.h>


 /* fifo size in elements (bytes) */
#define FIFO_SIZE       32


int test(void)
{

	struct kfifo fifo;
	int ret, tam;
	char *buf;

	/* Inicializo la cola y compruebo errores */
        ret = kfifo_alloc(&fifo, FIFO_SIZE, GFP_KERNEL);
        if (ret) {
                printk(KERN_ERR "error kfifo_alloc\n");
                return ret;
        }

	/* Introduzco la cadena Hello en la cola */
	kfifo_in(&fifo, "Hello", 5);

	/* Introduzco la cadena LIN de la cola */
	kfifo_in(&fifo, "LIN", 3);

	/* Extraigo la cadena Hello de la cola */
	tam = kfifo_out(&fifo, buf, 5);
	printk(KERN_INFO "%s", buf);

	/* Devuelvo la cadena LIN sin extraerla de la cola */
	if (kfifo_peek(&test, buf))
		printk(KERN_INFO " %s\n", buf);
	
	/* Elimino la cola */
	kfifo_free(&test);

	return 0;
}
