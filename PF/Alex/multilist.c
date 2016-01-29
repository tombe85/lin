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
const int MAX_NAME_LEN = 100;

typedef struct {
  int data;
  struct list_head links;
} list_item_t;

typedef struct {
    char *name;
    struct list_head *mylist;
    int listCount;
    struct list_head links;
} multilist_item_t;

struct list_head multilist;
int multilistCount;

struct semaphore multilist_lock;


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

static int listproc_open(struct inode *nodo, struct file *fich) {
    
    int i = 0;
    multilist_item_t *list;

    list = (multilist_item_t *) PDE_DATA(fich->f_inode); 

    return SUCCESS; 
}

static int listproc_release(struct inode *nodo, struct file *fich) {

    int i = 0;
    multilist_item_t *list;

    list = (multilist_item_t *) PDE_DATA(fich->f_inode);
	
    return SUCCESS;
}


/* Función write */
static ssize_t listcontrol_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
   
    int available_space = BUF_LEN-1;
    char kbuff[BUF_LEN];         //Copia de buffer de usuario
    char name[100];
    int ret;

    if (len > available_space || len > CBUF_SIZE) {
        printk(KERN_INFO "multilist: not enough space!!\n");
        return -ENOSPC;
    }

    /* Transfer data from user to kernel space */
    if (copy_from_user( &kbuff[0], buf, len ))
        return -EFAULT;

    kbuff[len] = '\0';  /* Add the '\0' */
    *off+=len;            /* Update the file pointer */

    /* AQUÍ CÓDIGO */

    if(sscanf(&kbuff[0],"create %s", name)) {
    
        /* Añadimos la nueva lista a la multilista */

        /* Informamos */
        printk(KERN_INFO "multilist: Added list called %s\n", name);

    } else if(sscanf(&kbuff[0],"delete %s", name)) {
        
        /* ELiminamos la lista de la multilista */

        /* Informamos */
        printk(KERN_INFO "multilist: Removed list called %s\n", name);

    } else {
        return -EINVAL;
    }

    return len;
}




int addList(char * name){
    multilist_item_t *nodo;
    int len = MAX_NAME_LEN;
    nodo = (multilist_item_t *)vmalloc(sizeof(multilist_item_t));
    nodo->name = (char *)vmalloc(len * sizeof(char));

    strcpy(nodo->name, name);

    /* Inicializamos la lista */
    INIT_LIST_HEAD(nodo->mylist);
    nodo->listCount = 0;

    if(down_interruptible(&multilist_lock)){
        return -ENAVAIL;
    }

    list_add_tail(&nodo->links, &multilist);
    multilistCount++;

    up(&multilist_lock);

    /* Creamos la entrada de proc */
    proc_entry = proc_create_data( name, 0666, proc_dir, &proc_entry_fops, (void *) nodo);
    if (proc_entry == NULL) {
        removeList(name, FALSE);
        printk(KERN_INFO "multilist: Can't create /proc entry\n");
        return -ENOMEM;
    }

    return 0;
}


int removeList(char *name, int remproc){
    struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;
    multilist_item_t *mynodo;
    int removed = -ENOENT;

    if(down_interruptible(&multilist_lock)){
        return -ENAVAIL;
    }

    /* Para cada nodo de la lista comprobamos si contiene num */
    list_for_each_safe(cur_node, aux, &multilist) {
        
        mynodo = list_entry(cur_node,multilist_item_t, links);

        if(strcmp(mynodo->name, name) == 0) {
            /* Eliminamos la lista */
            list_del(mynodo->mylist);
            /* Eliminamos de la lista */
            list_del(cur_node);
            /* Liberamos memoria dinámica */
            strcpy(name, mynodo->name);
            vfree(mynodo->name);
            vfree(mynodo);
            multilistCount--;
            removed = TRUE;
        }
    }

    up(&multilist_lock);

    if(removed && remproc)
        remove_proc_entry(name, proc_dir);

    return removed;
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
