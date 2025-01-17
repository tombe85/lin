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
int pares, impares;

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
static struct workqueue_struct * my_wq;	//wordqueue privada
my_work_t my_work;
// Lista enlazada
struct list_head linkedlistpar;
struct list_head linkedlistimpar;
typedef struct {
    unsigned int data;
    struct list_head links;
} list_item_t;
int linkedlistparcount;
int linkedlistimparcount;


/** Variables de sincronización y control **/
atomic_t numUsers;  // Controla que haya 1 consumidor solo
int consLockedpar;     // Indica si el consumidor está esperando a que haya elementos en la lista enlazada
int consLockedimpar;     // Indica si el consumidor está esperando a que haya elementos en la lista enlazada
DEFINE_SPINLOCK(spb);   // Para el buffer circular
struct semaphore semlistpar;   // Para el acceso a la lista enlazada
struct semaphore semlistimpar;   // Para el acceso a la lista enlazada
struct semaphore sembarrerapar;   // Barrera para bloquear al consumidor si no hay elementos en la lista par
struct semaphore sembarreraimpar;   // Barrera para bloquear al consumidor si no hay elementos en la lista impar
struct semaphore semreaders; // Semaforo para identificar los procesos en el open del timer

/* Entradas de proc */
static struct proc_dir_entry *proc_entry_cfg, *proc_entry_tmr;

/* Función que genera aleatorio al llamarla el timer */
static void generate_aleat(unsigned long data){
    unsigned int num;
    unsigned int threshold = (emergency_threshold * CBUF_SIZE) / 100;
    int act_cpu, tam, ret;
    unsigned long flags;
    int ticks = (int) (((float) timer_period / (float) 1000) * (float) HZ);

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
            flush_workqueue(my_wq);
            // Leemos la cpu de este proceso
            act_cpu = smp_processor_id();
            // lanzamos trabajo diferido en la wordqueue privada
            ret = queue_work(my_wq, (struct work_struct *)&my_work);
    	}
    }
    // reconfiguramos el timer
    mod_timer(&aleat_timer, jiffies + ticks);
}

// Función que añade un elemento a la lista enlazada.
void addListElement(unsigned int elem, list_item_t * nodo){
   
    nodo->data = elem;
    /* Pares */
    if((elem%2) == 0) {
		if(down_interruptible(&semlistpar)){
		   return;
		}
		list_add_tail(&nodo->links, &linkedlistpar);
		linkedlistparcount++;		
		up(&semlistpar);
		
	}
	else {
		if(down_interruptible(&semlistimpar)){
		   return;
		}
		list_add_tail(&nodo->links, &linkedlistimpar);
		linkedlistimparcount++;		
		up(&semlistimpar);
	
	}
}

// Función que vacía la lista enlazada pasada
void cleanUpList(struct list_head * list, struct semaphore * sem, int * count){
	list_item_t *mynodo;
	struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;

    if(down_interruptible(sem)){
        return;
    }

    list_for_each_safe(cur_node, aux, list) {
        mynodo = list_entry(cur_node,list_item_t, links);
        list_del(cur_node);
        vfree(mynodo);
    }
    (*count) = 0;
    up(sem);
}

/* Función para el trabajo diferido. Copia los elementos del buffer
 * circular a la lista enlazada y los borra del buffer
 */
static void copy_items_into_list( struct work_struct *work){
    my_work_t * mywork = (my_work_t *) work;
    unsigned int item[CBUF_SIZE];
    int i=0, tam;
    unsigned long flags;
    int haypar=0, hayimpar=0;

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
        if((item[i]%2)==0)
		haypar++;
	else
		hayimpar++;
	addListElement(item[i], nodo);
    }

    // Si el consumidor está bloqueado lo desbloqueamos
    if(haypar) {
	    if(consLockedpar > 0){
		    up(&sembarrerapar);
		    consLockedpar--;
	    }
	    printk(KERN_INFO "modtimer: Copied %d elements to the par list\n", haypar);
    }
    
    if(hayimpar) {
	    if(consLockedimpar > 0){
		    up(&sembarreraimpar);
		    consLockedimpar--;
	    }
	    printk(KERN_INFO "modtimer: Copied %d elements to the impar list\n", hayimpar);
    }
    
}

/** proc operations **/

static int modtimer_open(struct inode *nodo, struct file *fich){
    int ticks = (int) (((float) timer_period / (float) 1000) * (float) HZ);
    //Incrementamos el numero de consumidores
    if(down_interruptible(&semreaders)){
		return -EINTR;
    }
    atomic_inc(&numUsers);
    //Comprobamos que sólo pueda haber 1
    if(atomic_read(&numUsers) > 2){
        printk(KERN_INFO "modtimer: Error: Ya hay dos consumidor.");
        atomic_dec(&numUsers);
        up(&semreaders);
        return -ENOSPC;
    }
    
    /* Se les asigna el private_data correspondiente */
    if(atomic_read(&numUsers) == 1) {
	    pares = 1;
	    fich->private_data = (void *) &pares;
	    //controlamos que el módulo no pueda descargarse mientras haya consumidor
	    try_module_get(THIS_MODULE);
    }
    else {
	    impares = 2;
	    fich->private_data = (void *) &impares;
	    //Reconfiguramos el parámetro expires del timer y lo lanzamos
	    aleat_timer.expires = jiffies + ticks;
	    add_timer(&aleat_timer);
    }
    up(&semreaders);
    
    return SUCCESS;
}

static int modtimer_release(struct inode *nodo, struct file *fich){
    int *tipo = (int *)fich->private_data;
    int tipoval = *tipo;
    
    if(down_interruptible(&semreaders)){
	return -EINTR;
    }
    
    if(atomic_read(&numUsers) == 2){
	//desactivamos temporizador
	del_timer_sync(&aleat_timer);
	// esperamos a que acaben los trabajos diferidos
	flush_workqueue(my_wq);
	//vaciamos el buffer circular
	while(!is_empty_cbuffer_t(cbuffer)){
	    remove_cbuffer_t(cbuffer);
	}		   
    }else{
	 // Permitimos que el módulo pueda descargarse
	module_put(THIS_MODULE);		   
    }
    //Limpiamos las listas
    if(tipoval == 1) {
	// Si el semaforo está bloqueado lo desbloqueamos
	if(consLockedpar > 0){
	    up(&sembarrerapar);
	    consLockedpar--;
	}
	cleanUpList(&linkedlistpar, &semlistpar, &linkedlistparcount);		   
    }
    else {
	// Si el semaforo está bloqueado lo desbloqueamos
	if(consLockedimpar > 0){
	    up(&sembarreraimpar);
	    consLockedimpar--;
	}
	cleanUpList(&linkedlistimpar, &semlistimpar, &linkedlistimparcount);	   
    }
    //Decrementamos el número de consumidores
    atomic_dec(&numUsers);
    up(&semreaders);
	
    return SUCCESS;
}

static ssize_t modtimer_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    
    list_item_t *nodo;
    unsigned int dato;
    char kbuf[BUF_LEN];
    int *tipo = (int *)filp->private_data;
    int tipoval = *tipo;
    
    /* Pares */
    if(tipoval == 1) {
	if(linkedlistparcount == 0){   //No hay elementos en la lista
	    // Bloqueamos al consumidor hasta que haya elementos en la lista enlazada
	    consLockedpar++;
	    printk(KERN_INFO "modtimer: Consumidor par bloqueado. Esperando elementos\n");
	    if(down_interruptible(&sembarrerapar)){
		consLockedpar--;
		return -EINTR;
	    }
	}
	// Consumimos un elemento de la lista
	if(down_interruptible(&semlistpar)){
	    return -EINTR;
	}
	if(!list_empty(&linkedlistpar)){
		nodo = list_entry(linkedlistpar.next, list_item_t, links);
		dato = nodo->data;
		// Eliminamos el elemento de la lista
		list_del(linkedlistpar.next);  //Eliminamos el primer elemento de la lista
		linkedlistparcount--;
		vfree(nodo);
	}
	up(&semlistpar);
    }
    /* Impares */
    else {
	if(linkedlistimparcount == 0) {   //No hay elementos en la lista
	    // Bloqueamos al consumidor hasta que haya elementos en la lista enlazada
	    consLockedimpar++;
	    printk(KERN_INFO "modtimer: Consumidor impar bloqueado. Esperando elementos\n");
	    if(down_interruptible(&sembarreraimpar)){
		consLockedimpar--;
		return -EINTR;
	    }
	}
	// Consumimos un elemento de la lista
	if(down_interruptible(&semlistimpar)){
	    return -EINTR;
	}
	if(!list_empty(&linkedlistimpar)){
		nodo = list_entry(linkedlistimpar.next, list_item_t, links);
		dato = nodo->data;
		// Eliminamos el elemento de la lista
		list_del(linkedlistimpar.next);  //Eliminamos el primer elemento de la lista
		linkedlistimparcount--;
		vfree(nodo);
	}
	up(&semlistimpar);
    }
    
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
        timer_period = num;
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
    aleat_timer.expires = jiffies + (int) (((float) timer_period / (float) 1000) * (float) HZ);;
    aleat_timer.data = 0;
    aleat_timer.function = generate_aleat;
    
    
    my_wq = create_workqueue("my_wq");
    
    if(!my_wq) {
	return -ENOMEM;
    }	

    INIT_WORK((struct work_struct *) &my_work, copy_items_into_list);
    my_work.cbuffer = cbuffer;

    INIT_LIST_HEAD(&linkedlistpar);
    INIT_LIST_HEAD(&linkedlistimpar);
    linkedlistparcount = 0;
    linkedlistimparcount = 0;
    consLockedpar = 0;
    consLockedimpar = 0;

    sema_init(&semlistpar, 1);
    sema_init(&semlistimpar, 1);
    sema_init(&sembarrerapar, 0);
    sema_init(&sembarreraimpar, 0);
    sema_init(&semreaders, 1);

    printk(KERN_INFO "modtimer: Module loaded\n");
    return SUCCESS;
}

/* Función exit module */
void exit_modtimer_module( void )
{
    /* Eliminamos el buffer circular */
    destroy_cbuffer_t(cbuffer);
    
    /* Destruimos cola de trabajo */
	destroy_workqueue(my_wq);
	
    /* Eliminamos la entrada de proc */
    remove_proc_entry("modconfig", NULL);
    remove_proc_entry("modtimer", NULL);

    /* Informamos */
    printk(KERN_INFO "modtimer: Module unloaded.\n");
}

module_init( init_modtimer_module );
module_exit( exit_modtimer_module );
