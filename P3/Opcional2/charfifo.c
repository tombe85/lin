#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/ftrace.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include "cbuffer.h"

/* Datos del módulo */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Charfifo Character Device Module - FDI-UCM");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");

/* Constantes */
#define BUF_LEN 100
#define CBUF_SIZE 50
#define DEVICE_NAME "charfifo"
#define SUCCESS 0

/* Variables globales */
cbuffer_t *cbuffer;
static int Major; 
static int Device_Open = 0;
int prodcount = 0, conscount = 0;
struct semaphore mtx;
struct semaphore sem_prod;
struct semaphore sem_cons;
int nr_prod_waiting = 0;
int nr_cons_waiting = 0;



/* Prototipos */
int init_module( void );
void cleanup_module( void );
static int charfifo_open(struct inode *nodo, struct file *fich);
static int charfifo_release(struct inode *nodo, struct file *fich);
static ssize_t charfifo_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t charfifo_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);





/* Función write */
static ssize_t charfifo_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    int available_space = BUF_LEN-1;
    char kbuff[BUF_LEN];	     //Copia de buffer de usuario
    char * item;
    int i;

    /* The application can write in this entry just once !!
    if ((*off) > 0)
        return 0;*/

    if (len > available_space || len > CBUF_SIZE) {
        printk(KERN_INFO "fifoproc: not enough space!!\n");
        return -ENOSPC;
    }

    /* Transfer data from user to kernel space */
    if (copy_from_user( &kbuff[0], buf, len ))
        return -EFAULT;

    kbuff[len] = '\0'; 	/* Add the '\0' */
    *off+=len;            /* Update the file pointer */

    /* AQUÍ CÓDIGO */

    if (down_interruptible(&mtx)){
         return -EINTR;
    }

    while(CBUF_SIZE - size_cbuffer_t(cbuffer) < len && conscount > 0){
        nr_prod_waiting++;
        up(&mtx);
        if(down_interruptible(&sem_prod)){
            down(&mtx);
            nr_prod_waiting--;
            up(&mtx);
            return -EINTR;
        }

        if(down_interruptible(&mtx)){
            return -EINTR;
        }
    }

    if(conscount == 0){
        up(&mtx);
        return -EPIPE;
    }

    for(i=0; i < len; i++){
        item = vmalloc(sizeof(char));
        *(item) = kbuff[i];
        insert_cbuffer_t(cbuffer, item);
    }

    if(nr_cons_waiting > 0){
        up(&sem_cons);
        nr_cons_waiting--;
    }

    up(&mtx);

    return len;
}

/* Función read */
static ssize_t charfifo_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

    char kbuff[BUF_LEN];			    //buffer final
    char *item = NULL;
    int i;

    if(len > CBUF_SIZE || len > BUF_LEN){
		return -ENOSPC;
	}

    /* AQUÍ EL CÓDIGO */
    if(down_interruptible(&mtx)){
        return -EINTR;
    }

    while(size_cbuffer_t(cbuffer) < len && prodcount > 0){
        nr_cons_waiting++;

        up(&mtx);

        if(down_interruptible(&sem_cons)){
            down(&mtx);
            nr_cons_waiting--;
            up(&mtx);
            return -EINTR;
        }

        if(down_interruptible(&mtx)){
            return -EINTR;
        }
    }

    if(prodcount == 0){
        up(&mtx);
        return 0;
    }

    for(i=0; i < len; i++){
        item = head_cbuffer_t(cbuffer);
        remove_cbuffer_t(cbuffer);
        kbuff[i] = *item;
        vfree(item);
    }
    kbuff[i] = '\0';

    if(nr_prod_waiting > 0){
        up(&sem_prod);
        nr_prod_waiting--;
    }

    up(&mtx);

    /* Enviamos datos al espacio de ususario */
    if (copy_to_user(buf, kbuff, len))
        return -EINVAL;

    (*off)+=len;  /* Update the file pointer */

    /* Informamos */
    printk(KERN_INFO "fifoproc: Elements listed\n");


    return len;
}

static int charfifo_open(struct inode *nodo, struct file *fich){
    int i = 0;

    if (Device_Open)
        return -EBUSY;


    if (fich->f_mode & FMODE_READ){
        if(down_interruptible(&mtx)){
	       return -EINTR;
        }
        Device_Open++;
        conscount++;
        if(prodcount == 0){
            up(&mtx);
            if(down_interruptible(&sem_cons)){
                down(&mtx);
                conscount--;
                up(&mtx);
                return -EPIPE;
            }
            if(down_interruptible(&mtx)){
                return -EPIPE;
            }
        }
        if(prodcount > 0 && conscount == 1){    // estaban esperando un productor
            for(i=0; i < prodcount; i++)
                up(&sem_prod);
        }
        up(&mtx);
    }else{
        if(down_interruptible(&mtx)){
	       return -EINTR;
        }
        prodcount++;
        if(conscount == 0){
            up(&mtx);
            if(down_interruptible(&sem_prod)){
                down(&mtx);
                conscount--;
                up(&mtx);
                return -EPIPE;
            }
            if(down_interruptible(&mtx)){
                return -EPIPE;
            }
        }
        if(conscount > 0 && prodcount == 1){    // estaban esperando un productor
            for(i=0; i < conscount; i++)
                up(&sem_cons);
        }
        up(&mtx);
    }
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int charfifo_release(struct inode *nodo, struct file *fich){
    
    if (fich->f_mode & FMODE_READ){
        if(down_interruptible(&mtx)){
	       return -EINTR;
        }
        Device_Open--;
        conscount--;
        if(conscount == 0 && nr_prod_waiting > 0){
			int i, top = nr_prod_waiting;
			for(i=0; i < top; i++){
				up(&sem_prod);
				nr_prod_waiting--;
			}
		}
        up(&mtx);
    }else{
        if(down_interruptible(&mtx)){
	       return -EINTR;
        }
        prodcount--;
        if(prodcount == 0 && nr_cons_waiting > 0){
			int i, top = nr_cons_waiting;
			for(i=0; i < top; i++){
				up(&sem_cons);
				nr_cons_waiting--;
			}
		}
        up(&mtx);
    }
    if(down_interruptible(&mtx)){
		return -EINTR;
	}
	if(prodcount == 0 && conscount == 0){
		int size = size_cbuffer_t(cbuffer), i;
		for(i=0; i < size; i++){
			remove_cbuffer_t(cbuffer);
		}
	}
	up(&mtx);
    module_put(THIS_MODULE);
    return SUCCESS;
}



static const struct file_operations proc_entry_fops = {
    .read = charfifo_read,
    .write = charfifo_write,
    .open = charfifo_open,
    .release = charfifo_release,
};



/* Función init module */
int init_module( void )
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        printk(KERN_ALERT "Registering char fifo failed with %d\n", Major);
        return Major;
    }


    /* Inicializaciones */
    prodcount = 0;
    conscount = 0;
    sema_init(&mtx, 1);
    sema_init(&sem_cons, 0);
    sema_init(&sem_prod, 0);
    nr_prod_waiting = 0;
    nr_cons_waiting = 0;
    cbuffer = create_cbuffer_t(CBUF_SIZE);

    return SUCCESS;
}

/* Función exit module */
void cleanup_module( void )
{
    /* Eliminamos el dispositivo de tipo char */
    unregister_chrdev(Major, DEVICE_NAME);

    destroy_cbuffer_t(cbuffer);

    /* Informamos */
    printk(KERN_INFO "fifoproc: Module unloaded.\n");
}