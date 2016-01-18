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
#include <asm/atomic.h>
#include <asm/smp.h>
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
#define DEF_TICKS 250
#define DEF_THRESHOLD 80
#define DEF_MAXRDM 300

/* Variables globales */
int timer_period;
int max_random;
int emergency_threshold;

/** Componentes **/

// Timer
struct timer_list aleat_timer;
//Buffer circular
cbuffer_t *cbuffer;
//Trabajo diferido
typedef struct {
  struct work_struct work;
  cbuffer_t *cbuffer;   // apunta al buffer circular
} my_work_t;
my_work_t my_work;
// Lista enlazada
struct list_head linkedlist;
typedef struct {
    unsigned int data;
    struct list_head links;
} list_item_t;
int linkedlistcount;

/** Variables de sincronización y control **/
atomic_t numUsers;  // Controla que haya 1 consumidor solo
int consLocked;     // Indica si el consumidor está esperando a que haya elementos en la lista enlazada
DEFINE_SPINLOCK(spb);   // Para el buffer circular
struct semaphore semlist;   // Para el acceso a la lista enlazada
struct semaphore semcons;   // Barrera para bloquear al consumidor si no hay elementos en la lista enlazada

/* Entradas de proc */
static struct proc_dir_entry *proc_entry_cfg, *proc_entry_tmr;

/* Función que genera aleatorio al llamarla el timer */
static void generate_aleat(unsigned long data){
    unsigned int num;
    unsigned int threshold = (emergency_threshold * CBUF_SIZE) / 100;
    int act_cpu, tam;
    unsigned long flags;

    if(!is_full_cbuffer_t (cbuffer)){
        //Generamos número aleatorio
    	num = get_random_int() % max_random;

        // Insertamos número en el buffer y comprobamos su tamaño
    	spin_lock_irqsave(&spb, flags);
    	insert_cbuffer_t (cbuffer, num);
    	tam = size_cbuffer_t(cbuffer);
    	spin_unlock_irqrestore(&spb, flags);
        printk(KERN_INFO "modtimer: Generated %u\n", num);

        // Si se ha alcanzado el tamaño límite lanzamos trabajo diferido
    	if(tam >= threshold){
            //Esperamos a los trabajos previos
            if(work_pending((struct work_struct *) &my_work)){
                flush_scheduled_work();
            }
            // Leemos la cpu de este proceso
            act_cpu = smp_processor_id();
            // lanzamos trabajo diferido en la otra cpu
            schedule_work_on(act_cpu == 1 ? 0 : 1, (struct work_struct *) &my_work);
    	}
    }
    // reconfiguramos el timer
    mod_timer(&aleat_timer, jiffies + timer_period);
}

// Función que añade un elemento a la lista enlazada.
void addListElement(unsigned int elem, list_item_t * nodo){
    if(down_interruptible(&semlist)){
       return;
    }
    nodo->data = elem;
    list_add_tail(&nodo->links, &linkedlist);
    linkedlistcount++;
    up(&semlist);
}

// Función que vacía la lista enlazada
void cleanUpList(void){
	list_item_t *mynodo;
	struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;

    if(down_interruptible(&semlist)){
        return;
    }

    list_for_each_safe(cur_node, aux, &linkedlist) {
        mynodo = list_entry(cur_node,list_item_t, links);
        list_del(cur_node);
        vfree(mynodo);
    }
    linkedlistcount = 0;
    up(&semlist);
}

/* Función para el trabajo diferido. Copia los elementos del buffer
 * circular a la lista enlazada y los borra del buffer
 */
static void copy_items_into_list( struct work_struct *work){
    my_work_t * mywork = (my_work_t *) work;
    unsigned int item[CBUF_SIZE];
    int i=0, tam;
    unsigned long flags;

    // Guardamos los elementos del buffer en un array y vaciamos el buffer
    spin_lock_irqsave(&spb, flags);
    while(!is_empty_cbuffer_t(mywork->cbuffer)){
        item[i] = head_cbuffer_t(mywork->cbuffer);
        remove_cbuffer_t(mywork->cbuffer);
        i++;
    }
    spin_unlock_irqrestore(&spb, flags);
    //Nos quedamos con el tamaño del array de datos
    tam = i;

    // Añadimos los elementos leídos del buffer a la lista enlazada
    for(i=0; i < tam; i++){
        list_item_t * nodo;
        nodo = (list_item_t *)vmalloc(sizeof(list_item_t));
		addListElement(item[i], nodo);
    }

    // Si el consumidor está bloqueado lo desbloqueamos
    if(consLocked > 0){
        up(&semcons);
        consLocked--;
    }
    printk(KERN_INFO "modtimer: Copied %d elements to the list\n", tam);
}

/** proc operations **/

static int modtimer_open(struct inode *nodo, struct file *fich){
    //Incrementamos el numero de consumidores
    atomic_inc(&numUsers);
    //Comprobamos que sólo pueda haber 1
    if(atomic_read(&numUsers) > 1){
        printk(KERN_INFO "modtimer: Error: Ya hay un consumidor.");
        atomic_dec(&numUsers);
        return -ENOSPC; //TODO Ver error a lanzar
    }
    //Reconfiguramos el parámetro expires del timer y lo lanzamos
    aleat_timer.expires = jiffies + timer_period;
    add_timer(&aleat_timer);
    //controlamos que el módulo no pueda descargarse mientras haya consumidor
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int modtimer_release(struct inode *nodo, struct file *fich){
    // Si el semaforo está bloqueado lo desbloqueamos
    if(consLocked > 0){
        up(&semcons);
        consLocked--;
    }
    //desactivamos temporizador
    del_timer_sync(&aleat_timer);
    // esperamos a que acaben los trabajos diferidos
    if(work_pending((struct work_struct *) &my_work))
        flush_scheduled_work();
    //vaciamos el buffer circular
    while(!is_empty_cbuffer_t(cbuffer)){
        remove_cbuffer_t(cbuffer);
    }
    //Limpiamos la lista
    cleanUpList();
    //Decrementamos el número de consumidores
    atomic_dec(&numUsers);
    // Permitimos que el módulo pueda descargarse
    module_put(THIS_MODULE);
    return SUCCESS;
}

static ssize_t modtimer_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    list_item_t *nodo;
    unsigned int dato;
    char kbuf[BUF_LEN];

    if(linkedlistcount == 0){   //No hay elementos en la lista
        // Bloqueamos al consumidor hasta que haya elementos en la lista enlazada
        consLocked++;
        printk(KERN_INFO "modtimer: Consumidor bloqueado. Esperando elementos\n");
    	if(down_interruptible(&semcons)){
    	    consLocked--;
    	    return -EINTR;
    	}
    }
    // Consumimos un elemento de la lista
    if(down_interruptible(&semlist)){
        return -EINTR;
    }
    if(!list_empty(&linkedlist)){
        nodo = list_entry(linkedlist.next, list_item_t, links);
        dato = nodo->data;
        // Eliminamos el elemento de la lista
        list_del(linkedlist.next);  //Eliminamos el primer elemento de la lista
        linkedlistcount--;
        vfree(nodo);
    }
    up(&semlist);

    //Copiamos el elemento al espacio de usuario
    sprintf(&kbuf[0], "%u\n", dato);
    strcat(kbuf, "\0");
    if(copy_to_user(buf, kbuf, strlen(kbuf))){
	       return -EINVAL;
    }
    *off += strlen(kbuf);
    return strlen(kbuf);
}

static ssize_t modconfig_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    int nr_bytes;
    char kbuf[BUF_LEN];

    //Indicamos que no hay nada más que leer si ya se ha leído antes
    if ((*off) > 0)
        return 0;

    //Preparamos el texto de info de las variables globales
    kbuf[0] = '\0';
    sprintf(&kbuf[0], "timer_period_ms=%d\nemergency_threshold=%d\nmax_random=%d\n", timer_period, emergency_threshold, max_random);
    strcat(kbuf, "\0");
    nr_bytes=strlen(kbuf);
    if (len<nr_bytes)
        return -ENOSPC;
    // Mandamos la información al espacio de usuario
    if (copy_to_user(buf, kbuf, nr_bytes))
        return -EINVAL;

    (*off)+=len;
    return nr_bytes;
}

static ssize_t modconfig_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    int available_space = BUF_LEN-1;
    int num;
    char kbuf[BUF_LEN];

    if ((*off) > 0)
        return 0;
    if (len > available_space) {
        printk(KERN_INFO "modtimer: not enough space!!\n");
        return -ENOSPC;
    }
    // leemos la cadena de caracteres del espacio de usuario
    if (copy_from_user( &kbuf[0], buf, len ))
        return -EFAULT;

    kbuf[len] = '\0';
    *off+=len;

    /* Leemos ordenes y modificamos la variable especificada */
    if(sscanf(&kbuf[0],"timer_period_ms %i",&num)) {
        timer_period = (num/1000)*HZ; // pasamos de ms a tics
        printk(KERN_INFO "modtimer: changed timer_period_ms to %i\n", num);
    }else if(sscanf(&kbuf[0],"emergency_threshold %i",&num)) {
        emergency_threshold = num;
        printk(KERN_INFO "modtimer: changed emergency_threshold to %i\n", num);
    }else if(sscanf(&kbuf[0],"max_random %i",&num)) {
        max_random = num;
        printk(KERN_INFO "modtimer: changed max_random to %i\n", num);
    }else {
        return -EINVAL;
    }
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
    /* Creamos las entradas de proc */
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
    atomic_set(&numUsers, 0);
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

    printk(KERN_INFO "modtimer: Module loaded\n");
    return SUCCESS;
}

/* Función exit module */
void exit_modtimer_module( void )
{
    /* Eliminamos el buffer circular */
    destroy_cbuffer_t(cbuffer);

    /* Eliminamos la entrada de proc */
    remove_proc_entry("modconfig", NULL);
    remove_proc_entry("modtimer", NULL);

    /* Informamos */
    printk(KERN_INFO "modtimer: Module unloaded.\n");
}

module_init( init_modtimer_module );
module_exit( exit_modtimer_module );
