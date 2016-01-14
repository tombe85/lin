#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/ftrace.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/random.h>
#include <linux/workqueue.h>
#include "cbuffer.h"

/* Datos del módulo */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modtimer Kernel Module - FDI-UCM");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");

/* Constantes */
#define BUF_LEN 100
#define CBUF_SIZE 10
#ifndef SUCCESS
#define SUCCESS 0
#endif
#define DEF_TICKS 500
#define DEF_THRESHOLD 80
#define DEF_MAXRDM 300

/* Variables globales */
int timer_period;
int max_random;
int emergency_threshold;
cbuffer_t *cbuffer;
int numUsers;
struct timer_list aleat_timer;
typedef struct {
  struct work_struct work;
  cbuffer_t *cbuffer;
} my_work_t;
my_work_t my_work;
struct list_head linkedlist;
int linkedlistcount;
int consLocked;

typedef struct {
    unsigned int data;
    struct list_head links;
} list_item_t;

DEFINE_SPINLOCK(spb);

struct semaphore semlist;
struct semaphore semcons;


/* Entradas de proc */
static struct proc_dir_entry *proc_entry_cfg, *proc_entry_tmr;

/* Función que genera aleatorio al llamarla el timer */
static void generate_aleat(unsigned long data){
    unsigned int num;
    unsigned int ocup = (emergency_threshold * CBUF_SIZE) / 100;
    int act_cpu, tam;
    unsigned long flags;
    
    printk(KERN_INFO "modtimer: Usuarios %d\n", numUsers);

    if(numUsers > 0 && !is_full_cbuffer_t (cbuffer)){
    	num = get_random_int() % max_random;

    	spin_lock_irqsave(&spb, flags);
    	insert_cbuffer_t (cbuffer, num);
    	tam = size_cbuffer_t(cbuffer);
    	spin_unlock_irqrestore(&spb, flags);

    	if(tam >= ocup){
    	    act_cpu = smp_processor_id();

    	    /*if(work_pending((struct work_struct *) &my_work)){
    		          flush_work((struct work_struct *) &my_work);
    	    }*/
    	    flush_scheduled_work();
    	    
    	    if(act_cpu == 1){
    		    schedule_work_on(2, (struct work_struct *) &my_work);
    	    }else{
    		    schedule_work_on(1, (struct work_struct *) &my_work);
    	    }
    	}
		printk(KERN_INFO "modtimer: Generated %u\n", num);
    }
    
    mod_timer(&aleat_timer, jiffies + timer_period);
}

// Semaforo ya está bloqueado
void addListElement(unsigned int elem){
    list_item_t * nodo;

    nodo = (list_item_t *)vmalloc(sizeof(list_item_t));

    nodo->data = elem;

    list_add_tail(&nodo->links, &linkedlist);
    linkedlistcount++;
}

static void copy_items_into_list( struct work_struct *work){
    my_work_t * mywork = (my_work_t *) work;
    unsigned int *item;
    int i=0;
    int tam = (emergency_threshold * CBUF_SIZE) / 100;
    unsigned long flags;

    item = (unsigned int *)vmalloc(sizeof(unsigned int)*tam);

    spin_lock_irqsave(&spb, flags);
    while(!is_empty_cbuffer_t(mywork->cbuffer) && i < tam){
        item[i] = head_cbuffer_t(mywork->cbuffer);
        remove_cbuffer_t(mywork->cbuffer);
        i++;
    }
    spin_unlock_irqrestore(&spb, flags);
    
    if(down_interruptible(&semlist)){
	       return;
    }
    
    for(i=0; i < tam; i++){
		addListElement((item[i]));
		//vfree(item[i]);
    }

    while(consLocked > 0){
        up(&semcons);
        consLocked--;
    }

    up(&semlist);
    vfree(item);
    printk(KERN_INFO "modtimer: Copied %d elements to the list\n", tam);
}

static int modtimer_open(struct inode *nodo, struct file *fich){
    numUsers++;
    return SUCCESS;
}

static int modtimer_release(struct inode *nodo, struct file *fich){
    numUsers--;
    printk(KERN_INFO "modtimer: Ejecutado release. Users: %d\n", numUsers);
    return SUCCESS;
}

static ssize_t modtimer_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    list_item_t *nodo;
    unsigned int dato;
    char kbuf[BUF_LEN];

    if(linkedlistcount == 0){
        consLocked++;
    	if(down_interruptible(&semcons)){
    	    consLocked--;
    	    return -EINTR;
    	}

    }
    if(down_interruptible(&semlist)){
        return -EINTR;
    }
    nodo = list_entry(linkedlist.next, list_item_t, links);
    dato = nodo->data;
    list_del(linkedlist.next);
    linkedlistcount--;
    vfree(nodo);
    up(&semlist);

    sprintf(kbuf, "%u\n\0", dato);
    if(copy_to_user(buf, kbuf, strlen(kbuf))){
	       return -EINVAL;
    }

    *off += strlen(kbuf);

    return strlen(kbuf);
}

static ssize_t modconfig_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    return len;
}

static ssize_t modconfig_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    return len;
}

static const struct file_operations proc_entry_fops_cfg = {
    .read = modconfig_read,
    .write = modconfig_write,
};

static const struct file_operations proc_entry_fops_tmr = {
    .read = modtimer_read,
    .open = modtimer_open,
    .release = modtimer_release,
};

/* Función init module */
int init_modtimer_module( void )
{
    /* Creamos la entrada de proc */
    proc_entry_cfg = proc_create( "modconfig", 0666, NULL, &proc_entry_fops_cfg);
    if (proc_entry_cfg == NULL) {
        printk(KERN_INFO "modtimer: Can't create /proc entry\n");
        return -ENOMEM;
    } else {
    	proc_entry_tmr = proc_create( "modtimer", 0666, NULL, &proc_entry_fops_tmr);
    	if(proc_entry_tmr == NULL) {
    	    printk(KERN_INFO "modtimer: Can't create /proc entry\n");
    	    remove_proc_entry("modconfig", NULL);
    	    return -ENOMEM;
    	}
    }

    /* Inicializaciones */
    cbuffer = create_cbuffer_t(CBUF_SIZE);
    numUsers = 0;
    /* Create timer */
    init_timer(&aleat_timer);
    timer_period = DEF_TICKS;
    max_random = DEF_MAXRDM;
    emergency_threshold = DEF_THRESHOLD;
    aleat_timer.expires = jiffies + timer_period;
    aleat_timer.data = 0;
    aleat_timer.function = generate_aleat;

    INIT_WORK((struct work_struct *) &my_work, copy_items_into_list);
    my_work.cbuffer = cbuffer;

    INIT_LIST_HEAD(&linkedlist);
    linkedlistcount = 0;
    consLocked = 0;

    sema_init(&semlist, 1);
    sema_init(&semcons, 0);
    
    printk(KERN_INFO "modtimer: Se va a iniciar el timer\n");

    /* Acciones */
    add_timer(&aleat_timer);

    printk(KERN_INFO "modtimer: Module loaded\n");
    return SUCCESS;
}

void cleanUpList(void){
	list_item_t *mynodo;
	struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;

    down_interruptible(&semlist);

    /* Recorremos la lista */
    list_for_each_safe(cur_node, aux, &linkedlist) {
        mynodo = list_entry(cur_node,list_item_t, links);

        /* Eliminamos nodo de la lista */
        list_del(cur_node);

        /*Liberamos memoria dinámica del nodo */
        vfree(mynodo);
    }
    linkedlistcount = 0;

    up(&semlist);
}

/* Función exit module */
void exit_modtimer_module( void )
{

    del_timer_sync(&aleat_timer);
    flush_scheduled_work();

    /* Eliminamos la entrada de proc */
    remove_proc_entry("modconfig", NULL);
    remove_proc_entry("modtimer", NULL);

    destroy_cbuffer_t(cbuffer);

    cleanUpList();

    /* Informamos */
    printk(KERN_INFO "modtimer: Module unloaded.\n");
}

module_init( init_modtimer_module );
module_exit( exit_modtimer_module );
