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
#define MAX_NAME_LEN 30;

/* Variables globales */
struct list_head fifolist;
int listFifoCount;
typedef struct {
	char name[MAX_NAME_LEN];
	cbuffer_t *cbuffer;
	int prodcount = 0;
	int conscount = 0;
	struct semaphore mtx;
	struct semaphore sem_prod;
	struct semaphore sem_cons;
	int nr_prod_waiting = 0;
	int nr_cons_waiting = 0;
	struct list_head links;
} fifolist_item_t;
DEFINE_SPINLOCK(fifolist_lock);

/* Entrada de proc */
static struct proc_dir_entry *proc _dir;
static struct proc_dir_entry *proc_entry;
static struct proc_dir_entry *proc_control;

/* Función write */
static ssize_t fifoproc_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    int available_space = BUF_LEN-1;
    char kbuff[BUF_LEN];	     //Copia de buffer de usuario
    char item;
    int i;
	fifolist_item_t *fifo, *nodo;
	struct list_head *cur_node = NULL;

    char *fifoname = (char *) PDE_DATA(filp->f_inode);

	list_for_each(cur_node, &fifolist){
		nodo = list_entry(cur_node,fifolist_item_t, links);
		if(strcmp(nodo->name, fifoname)){
			fifo = nodo;
		}
	}

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

    if (down_interruptible(&(fifo->mtx))){
         return -EINTR;
    }

    while(CBUF_SIZE - size_cbuffer_t(fifo->cbuffer) < len && fifo->conscount > 0){
        fifo->nr_prod_waiting++;
        up(&(fifo->mtx));
        if(down_interruptible(&(fifo->sem_prod))){
            down(&(fifo->mtx));
            fifo->nr_prod_waiting--;
            up(&(fifo->mtx));
            return -EINTR;
        }

        if(down_interruptible(&(fifo->mtx))){
            return -EINTR;
        }
    }

    if(fifo->conscount == 0){
        up(&(fifo->mtx));
        return -EPIPE;
    }

    for(i=0; i < len; i++){
        item = kbuff[i];
        insert_cbuffer_t(fifo->cbuffer, item);
    }

    if(fifo->nr_cons_waiting > 0){
        up(&(fifo->sem_cons));
        fifo->nr_cons_waiting--;
    }

    up(&(fifo->mtx));

    return len;
}

/* Función read */
static ssize_t fifoproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

    char kbuff[BUF_LEN];			    //buffer final
    char *item = NULL;
    int i;

	fifolist_item_t *fifo, *nodo;
	struct list_head *cur_node = NULL;

    char *fifoname = (char *) PDE_DATA(filp->f_inode);

	list_for_each(cur_node, &fifolist){
		nodo = list_entry(cur_node,fifolist_item_t, links);
		if(strcmp(nodo->name, fifoname)){
			fifo = nodo;
		}
	}

    if(len > CBUF_SIZE || len > BUF_LEN){
		return -ENOSPC;
	}

    /* AQUÍ EL CÓDIGO */
    if(down_interruptible(&(fifo->mtx))){
        return -EINTR;
    }

    while(size_cbuffer_t(fifo->cbuffer) < len && fifo->prodcount > 0){
        fifo->nr_cons_waiting++;

        up(&(fifo->mtx));

        if(down_interruptible(&(fifo->sem_cons))){
            down(&(fifo->mtx));
            fifo->nr_cons_waiting--;
            up(&(fifo->mtx));
            return -EINTR;
        }

        if(down_interruptible(&(fifo->mtx))){
            return -EINTR;
        }
    }

    if(fifo->prodcount == 0){
        up(&(fifo->mtx));
        return 0;
    }

    for(i=0; i < len; i++){
        item = head_cbuffer_t(fifo->cbuffer);
		kbuff[i] = *item;
        remove_cbuffer_t(fifo->cbuffer);
    }
    kbuff[i] = '\0';

    if(fifo->nr_prod_waiting > 0){
        up(&(fifo->sem_prod));
        fifo->nr_prod_waiting--;
    }

    up(&(fifo->mtx));

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
	fifolist_item_t *fifo = NULL, *nodo;
	struct list_head *cur_node = NULL;

    char *fifoname = (char *) PDE_DATA(fich->f_inode);

	list_for_each(cur_node, &fifolist){
		nodo = list_entry(cur_node,fifolist_item_t, links);
		if(strcmp(nodo->name, fifoname)){
			fifo = nodo;
		}
	}

    if (fich->f_mode & FMODE_READ){
        if(down_interruptible(&(fifo->mtx))){
	       return -EINTR;
        }
        fifo->conscount++;
        if(fifo->prodcount == 0){
            up(&(fifo->mtx));
            if(down_interruptible(&(fifo->sem_cons))){
                down(&(fifo->mtx));
                fifo->conscount--;
                up(&(fifo->mtx));
                return -EPIPE;
            }
            if(down_interruptible(&(fifo->mtx))){
                return -EPIPE;
            }
        }
        if(fifo->prodcount > 0 && fifo->conscount == 1){    // estaban esperando un productor
            for(i=0; i < fifo->prodcount; i++)
                up(&(fifo->sem_prod));
        }
        up(&(fifo->mtx));
    }else{
        if(down_interruptible(&(fifo->mtx))){
	       return -EINTR;
        }
        fifo->prodcount++;
        if(fifo->conscount == 0){
            up(&(fifo->mtx));
            if(down_interruptible(&(fifo->sem_prod))){
                down(&(fifo->mtx));
                fifo->conscount--;
                up(&(fifo->mtx));
                return -EPIPE;
            }
            if(down_interruptible(&(fifo->mtx))){
                return -EPIPE;
            }
        }
        if(fifo->conscount > 0 && fifo->prodcount == 1){    // estaban esperando un productor
            for(i=0; i < fifo->conscount; i++)
                up(&(fifo->sem_cons));
        }
        up(&(fifo->mtx));
    }
    return SUCCESS;
}

static int fifoproc_release(struct inode *nodo, struct file *fich){
	fifolist_item_t *fifo = NULL, *nodo;
	struct list_head *cur_node = NULL;

    char *fifoname = (char *) PDE_DATA(fich->f_inode);

	list_for_each(cur_node, &fifolist){
		nodo = list_entry(cur_node,fifolist_item_t, links);
		if(strcmp(nodo->name, fifoname)){
			fifo = nodo;
		}
	}

    if (fich->f_mode & FMODE_READ){
        if(down_interruptible(&(fifo->mtx))){
	       return -EINTR;
        }
        fifo->conscount--;
        if(fifo->conscount == 0 && fifo->nr_prod_waiting > 0){
			int i, top = fifo->nr_prod_waiting;
			for(i=0; i < top; i++){
				up(&(fifo->sem_prod));
				fifo->nr_prod_waiting--;
			}
		}
        up(&(fifo->mtx));
    }else{
        if(down_interruptible(&(fifo->mtx))){
	       return -EINTR;
        }
        fifo->prodcount--;
        if(fifo->prodcount == 0 && fifo->nr_cons_waiting > 0){
			int i, top = fifo->nr_cons_waiting;
			for(i=0; i < top; i++){
				up(&(fifo->sem_cons));
				fifo->nr_cons_waiting--;
			}
		}
        up(&(fifo->mtx));
    }
    if(down_interruptible(&(fifo->mtx))){
		return -EINTR;
	}
	if(fifo->prodcount == 0 && fifo->conscount == 0){
		int size = size_cbuffer_t(fifo->cbuffer), i;
		for(i=0; i < size; i++){
			remove_cbuffer_t(fifo->cbuffer);
		}
	}
	up(&(fifo->mtx));
    return SUCCESS;
}


/* Función write */
static ssize_t fifocontrol_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    int available_space = BUF_LEN-1;
    char kbuff[BUF_LEN];	     //Copia de buffer de usuario
    int i;
	char name[MAX_NAME_LEN];

    if (len > available_space || len > CBUF_SIZE) {
        printk(KERN_INFO "fifomulti: not enough space!!\n");
        return -ENOSPC;
    }

    /* Transfer data from user to kernel space */
    if (copy_from_user( &kbuff[0], buf, len ))
        return -EFAULT;

    kbuff[len] = '\0'; 	/* Add the '\0' */
    *off+=len;            /* Update the file pointer */

    /* AQUÍ CÓDIGO */

	if(sscanf(&kbuff[0],"create %s", name)) {
        if(addFifo(name)){
			return -ENOMEM;
		}

        /* Informamos */
        printk(KERN_INFO "fifomulti: Added fifo called \n", name);

    }else if(sscanf(&kbuff[0],"delete %s", name)){
		if(!removeFifo(name, TRUE)){
			return -ENOENT;
		}
		/* Informamos */
        printk(KERN_INFO "fifomulti: Removed fifo called \n", name);
	}else{
		return -EINVAL;
	}

    return len;
}


static const struct file_operations proc_entry_fops = {
    .read = fifoproc_read,
    .write = fifoproc_write,
    .open = fifoproc_open,
    .release = fifoproc_release,
};

static const struct file_operations proc_control_fops = {
    .write = fifocontrol_write,
};

int addFifo(char * name){
	fifolist_item_t *nodo;

	nodo = (fifolist_item_t *)vmalloc(sizeof(fifolist_item_t));

	spin_lock(&fifolist_lock);

	strcpy(nodo->name, name);
	nodo->prodcount = 0;
    nodo->conscount = 0;
    sema_init(&(nodo->mtx), 1);
    sema_init(&(nodo->sem_cons), 0);
    sema_init(&(nodo->sem_prod), 0);
    nodo->nr_prod_waiting = 0;
    nodo->nr_cons_waiting = 0;
    nodo->cbuffer = create_cbuffer_t(CBUF_SIZE);

	list_add_tail(&nodo->links, &fifolist);
	listFifoCount++;

	spin_unlock(&fifolist_lock);

	/* Creamos la entrada de proc */
    proc_entry = proc_create_data( name, 0666, proc_dir, &proc_entry_fops, (void *) nodo->name);
    if (proc_entry == NULL) {
		removeFifo(name, FALSE);
        printk(KERN_INFO "fifoproc: Can't create /proc entry\n");
        return -ENOMEM;
    }

	return 0;
}

int removeFifo(char *name, int remproc){
    struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;
    fifolist_item_t *mynodo;
	int removed = FALSE;

    spin_lock(&fifolist_lock);

    /* Para cada nodo de la lista comprobamos si contiene num */
    list_for_each_safe(cur_node, aux, &fifolist) {
        mynodo = list_entry(cur_node,list_item_t, links);
        if(strcmp(mynodo->name, name) == 0) {
			/* Eliminamos buffer */
			destroy_cbuffer_t(mynodo->cbuffer);
            /* Eliminamos de la lista */
            list_del(cur_node);
            /* Liberamos memoria dinámica */
            vfree(mynodo);
            listFifoCount--;
			removed = TRUE;
        }
    }

    spin_unlock(&fifolist_lock);
	if(removed && remproc)
		remove_proc_entry(name, proc_dir);

	return removed;
}

/* Función init module */
int init_fifoproc_module( void )
{
	int ret;
	/* Creamos directorio proc */
	proc_dir = proc_mkdir("fifomulti", NULL);
	if (proc_dir == NULL) {
        printk(KERN_INFO "fifomulti: Can't create /proc dir\n");
        return -ENOMEM;
    }
	proc_control = proc_create( "control", 0666, proc_dir, &proc_control_fops);
    if (proc_entry == NULL) {
		remove_proc_entry("fifomulti", NULL);
        printk(KERN_INFO "fifomulti: Can't create /proc entry\n");
        return -ENOMEM;
    }

    /* Inicializaciones */
	INIT_LIST_HEAD(&mylist);
	listFifoCount = 0;
	if(ret = addFifo("default\0"))
		return ret;

	printk(KERN_INFO "fifomulti: Module loaded\n");

    return 0;
}

/* Función exit module */
void exit_fifoproc_module( void )
{
	free_fifos();
    /* Eliminamos la entrada de proc */
	remove_proc_entry("control", proc_dir);
    remove_proc_entry("fifomulti", NULL);	//Directorio

    /* Informamos */
    printk(KERN_INFO "fifoproc: Module unloaded.\n");
}


module_init( init_fifoproc_module );
module_exit( exit_fifoproc_module );
