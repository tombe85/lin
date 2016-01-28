#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/ftrace.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>

/* Datos del módulo */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MultiListproc Module - FDI-UCM");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");


/* Variables globales */
struct list_head multilist;
int multilistCount;


/* Entrada de proc */
static struct proc_dir_entry *proc _dir;
static struct proc_dir_entry *proc_entry;
static struct proc_dir_entry *proc_control;

/* Función write */
static ssize_t listproc_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    
}

/* Función read */
static ssize_t listproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

   
}

static int listproc_open(struct inode *nodo, struct file *fich){
   
}

static int listproc_release(struct inode *nodo, struct file *fich){
	
    return SUCCESS;
}


/* Función write */
static ssize_t listcontrol_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
   
}


static const struct file_operations proc_entry_fops = {
    .read = listproc_read,
    .write = listproc_write,
    .open = listproc_open,
    .release = listproc_release,
};

static const struct file_operations proc_control_fops = {
    .write = listcontrol_write,
};


/* Función init module */
int init_listproc_module( void )
{
	int ret;
	/* Creamos directorio proc */
	proc_dir = proc_mkdir("listmulti", NULL);
	if (proc_dir == NULL) {
        printk(KERN_INFO "listmulti: Can't create /proc dir\n");
        return -ENOMEM;
    }
	proc_control = proc_create( "control", 0666, proc_dir, &proc_control_fops);
    if (proc_entry == NULL) {
		remove_proc_entry("listmulti", NULL);
        printk(KERN_INFO "listmulti: Can't create /proc entry\n");
        return -ENOMEM;
    }

    /* Inicializaciones */
	INIT_LIST_HEAD(&mylist);
    /* Añadir nodo default a la lista */

	printk(KERN_INFO "listmulti: Module loaded\n");

    return 0;
}

/* Función exit module */
void exit_listproc_module( void )
{
	free_fifos();
    /* Eliminamos la entrada de proc */
	remove_proc_entry("control", proc_dir);
    remove_proc_entry("listmulti", NULL);	//Directorio

    /* Informamos */
    printk(KERN_INFO "listproc: Module unloaded.\n");
}


module_init( init_listproc_module );
module_exit( exit_listproc_module );
