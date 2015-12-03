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

    /* The application can write in this entry just once !! */
    if ((*off) > 0)
        return 0;

    if (len > available_space) {
        printk(KERN_INFO "fifoproc: not enough space!!\n");
        return -ENOSPC;
    }

    /* Transfer data from user to kernel space */
    if (copy_from_user( &kbuff[0], buf, len ))
        return -EFAULT;

    kbuff[len] = '\0'; 	/* Add the '\0' */
    *off+=len;            /* Update the file pointer */

    /* AQUÍ CÓDIGO */

    return len;
}

/* Función read */
static ssize_t fifoproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

    int nr_bytes;					    // Bytes leídos
    char kbuff[BUF_LEN];			    //buffer final
    kbuff[0] = '\0';

    if ((*off) > 0)
        return 0;

    /* AQUÍ EL CÓDIGO */


    /* Cargamos los bytes leídos */
    nr_bytes=strlen(kbuff);

    /* Enviamos datos al espacio de ususario */
    if (copy_to_user(buf, kbuff, nr_bytes))
        return -EINVAL;

    (*off)+=len;  /* Update the file pointer */

    /* Informamos */
    printk(KERN_INFO "fifoproc: Elements listed\n");

    return nr_bytes;
}

static int fifoproc_open(struct inode nodo*, struct file fich*){
    // ...
    if (fich->f_mode & FMODE_READ){
        /* Un consumidor abrió el FIFO */

    }else{
        /* Un productor abrió el FIFO */

    }
}

static int fifoproc_release(struct inode nodo*, struct file fich*){
    // ...
    if (fich->f_mode & FMODE_READ){
        /* Un consumidor cerró el FIFO */

    }else{
        /* Un productor cerró el FIFO */

    }
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
    sema_init(&sem_cons, 0);    //??
    sema_init(&sem_prod, 1);    //??
    nr_prod_waiting = 0;
    nr_cons_waiting = 0;

    return 0;
}

/* Función exit module */
void exit_fifoproc_module( void )
{
    /* Eliminamos la entrada de proc */
    remove_proc_entry("fifoproc", NULL);

    /* Informamos */
    printk(KERN_INFO "fifoproc: Module unloaded.\n");
}


module_init( init_fifoproc_module );
module_exit( exit_fifoproc_module );
