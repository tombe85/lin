#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/ftrace.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include "cbuffer.h"

/* Datos del módulo */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fifoproc Module - FDI-UCM");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");

/* Constantes */
#define BUF_LEN 100
#define CBUF_SIZE 50
#define SUCCESS 0

/* Variables globales */
cbuffer_t *cbuffer;
int prodcount = 0, conscount = 0;
struct semaphore mtx;
struct semaphore sem_prod;
struct semaphore sem_cons;
int nr_prod_waiting = 0;
int nr_cons_waiting = 0;

/* Entrada de proc */
static struct proc_dir_entry *proc_entry;

/* Función write */
static ssize_t fifoproc_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
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
static ssize_t fifoproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

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

static int fifoproc_open(struct inode *nodo, struct file *fich){
    int i = 0;
    if (fich->f_mode & FMODE_READ){
        if(down_interruptible(&mtx)){
	       return -EINTR;
        }
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
    return SUCCESS;
}

static int fifoproc_release(struct inode *nodo, struct file *fich){
    if (fich->f_mode & FMODE_READ){
        if(down_interruptible(&mtx)){
	       return -EINTR;
        }
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
    return SUCCESS;
}

static const struct file_operations proc_entry_fops = {
    .read = fifoproc_read,
    .write = fifoproc_write,
    .open = fifoproc_open,
    .release = fifoproc_release,
};

/* Función init module */
int init_fifoproc_module( void )
{
    /* Creamos la entrada de proc */
    proc_entry = proc_create( "fifoproc", 0666, NULL, &proc_entry_fops);
    if (proc_entry == NULL) {
        printk(KERN_INFO "fifoproc: Can't create /proc entry\n");
        return -ENOMEM;
    } else {
        printk(KERN_INFO "fifoproc: Module loaded\n");
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

    return 0;
}

/* Función exit module */
void exit_fifoproc_module( void )
{
    /* Eliminamos la entrada de proc */
    remove_proc_entry("fifoproc", NULL);

    destroy_cbuffer_t(cbuffer);

    /* Informamos */
    printk(KERN_INFO "fifoproc: Module unloaded.\n");
}


module_init( init_fifoproc_module );
module_exit( exit_fifoproc_module );
