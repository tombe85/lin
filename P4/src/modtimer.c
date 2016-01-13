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

typedef struct {
    int data;
    struct list_head links;
} list_item_t;

/* Entradas de proc */
static struct proc_dir_entry *proc_entry_cfg, *proc_entry_tmr;


/* Función que genera aleatorio al llamarla el timer */
void generate_aleat(void){
    unsigned int *num;
    unsigned int ocup = (emergency_threshold * CBUF_SIZE) / 100;
    int act_cpu;
    num = (unsigned int) kmalloc(sizeof(unsigned int));
    *num = get_random_int() % max_random;
    insert_cbuffer_t (cbuffer, (void *) num);
    if(size_cbuffer_t(cbuffer) >= ocup){
	act_cpu = smp_processor_id();
	
	if(work_pending((struct work_struct *) my_work)){
	    flush_work((struct work_struct *) my_work);
	}
	//TODO : Vaciado de buffer
	if(!act_cpu){
	    schedule_work_on(1, (struct work_struct *) my_work);
	}else{
	    schedule_work_on(0, (struct work_struct *) my_work);
	}
	
	
    }
    mod_timer(&aleat_timer, jiffies + timer_period);
}

static void copy_items_into_list( struct work_struct *work){
    my_work_t * mywork = (my_work_t *) work;
    
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
    timer_period = DEF_TICKS;
    max_random = DEF_MAXRDM;
    emergency_threshold = DEF_THRESHOLD;
    aleat_timer.expires = jiffies + timer_period;
    aleat_timer.data = 0;
    aleat_timer.function = generate_aleat;
    INIT_WORK((struct work_struct *) &my_work, copy_items_into_list);
    
    /* Acciones */
    add_timer(&aleat_timer);
    
    printk(KERN_INFO "modtimer: Module loaded\n");
    return SUCCESS;
}

/* Función exit module */
void exit_modtimer_module( void )
{
    /* Eliminamos la entrada de proc */
    remove_proc_entry("fifoproc", NULL);

    destroy_cbuffer_t(cbuffer);

    /* Informamos */
    printk(KERN_INFO "fifoproc: Module unloaded.\n");
}

module_init( init_modtimer_module );
module_exit( exit_modtimer_module );
