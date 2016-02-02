#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/sort.h>
#include <asm-generic/uaccess.h>
#include <linux/ftrace.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>

/* Datos del módulo */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MultiListproc Module - FDI-UCM");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");

/* Constantes */
#define BUF_LEN 100
#define TRUE 1
#define FALSE 0

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
    spinlock_t lock; //void spin_lock_init(spinlock_t *lock);
    struct list_head links;
} multilist_item_t;

struct list_head multilist;
int multilistCount;

struct semaphore multilist_lock;


/* Entrada de proc */
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;
static struct proc_dir_entry *proc_control;



/* Prototipos */
int addList(char * name);
int removeList(char *name, int remproc);
int cmpElement(const void *a, const void *b);
void swapp(void * a, void * b, int size);
void sortlist(multilist_item_t *lista);
void addElement(int num, multilist_item_t *lista);
void removeElement(int num, multilist_item_t *lista);
void cleanUpList(multilist_item_t *lista);
static ssize_t listproc_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static ssize_t listproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t listcontrol_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);



static const struct file_operations proc_entry_fops = {
    .read = listproc_read,
    .write = listproc_write,
};

static const struct file_operations proc_control_fops = {
    .write = listcontrol_write,
};




/* Función write */
static ssize_t listproc_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    int available_space = BUF_LEN-1;
    int num;
    char kbuff[BUF_LEN];         //Copia de buffer de usuario
    char procstr[BUF_LEN];
    multilist_item_t *lista;


    lista = (multilist_item_t *) PDE_DATA(filp->f_inode);

    /* The application can write in this entry just once !! */
    if ((*off) > 0)
        return 0;


    if (len > available_space) {
        printk(KERN_INFO "Modlist: not enough space!!\n");
        return -ENOSPC;
    }

    /* Transfer data from user to kernel space */
    if (copy_from_user( &kbuff[0], buf, len ))
        return -EFAULT;

    kbuff[len] = '\0';  /* Add the '\0' */
    *off+=len;            /* Update the file pointer */

    /* Leemos ordenes */
    if(sscanf(&kbuff[0],"add %i",&num)) {
        addElement(num, lista);
        /* Informamos */
        printk(KERN_INFO "Multilist: Added element %i to the list %s\n", num, lista->name);

    } else if(sscanf(&kbuff[0],"remove %i",&num)) {
        /* Si la lista no esta vacia */
        if(list_empty(lista->mylist) == 0) {
            removeElement(num, lista);

            /* Informamos */
            printk(KERN_INFO "Multilist: Element %i deleted from list, %s\n", num, lista->name);
        } else
            printk(KERN_INFO "Multilist: Can't remove element %i. The list %s empty\n", num, lista->name);

    } else if(sscanf(&kbuff[0],"%s", procstr)) {
       
        if(strcmp("cleanup", procstr)==0) {
            /* Si la lista no esta vacia */
            if(list_empty(lista->mylist) == 0) {
                /* Limpiamos la lista entera */
                cleanUpList(lista);
                /* Informamos */
                printk(KERN_INFO "Multilist: List %s cleaned up\n", lista->name);
            } else
                printk(KERN_INFO "Multilist: Can't cleanup list. The list %s is empty\n", lista->name);
        } else if(strcmp("sort", procstr) == 0) {
            /* Si la lista no esta vacia */
            if(list_empty(lista->mylist) == 0) {
                /* Ordenamos cadena */
                sortlist(lista);
                printk(KERN_INFO "Multilist: List %s sorted\n", lista->name);
            } else
                printk(KERN_INFO "Multilist: Can't sort list. The list %s is empty\n", lista->name);
        }
    }
    else {
        // retornar un codigo de error correspondiente (instruccion incorrecta)
        return -EINVAL;
    }
    return len; 
}

/* Función read */
static ssize_t listproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

    int nr_bytes;                       // Bytes leídos
    list_item_t *item=NULL;             //Nodo a leer
    struct list_head* cur_node=NULL;
    char kbuff[BUF_LEN];                //buffer final
    char strtmp[BUF_LEN];               //cadenas temporales
    int cerrojo = 1;
    multilist_item_t *lista;
    
    kbuff[0] = '\0';


    lista = (multilist_item_t *) PDE_DATA(filp->f_inode);

    if ((*off) > 0) /* Tell the application that there is nothing left to read */
        return 0;

    /* Listamos los elementos de la lista */
    spin_lock(&(lista->lock));

    list_for_each(cur_node, lista->mylist) {
        
        item = list_entry(cur_node,list_item_t, links);
        /* Escribimos en msgtmp el dato, el \n y lo concatenamos al kbuff final */
        sprintf(strtmp, "%i\n", item->data);

        if(strlen(kbuff) < BUF_LEN - strlen(strtmp) && cerrojo) {
            strcat(kbuff, strtmp);
        } else {
            cerrojo = 0;
        }
    }
    
    spin_unlock(&(lista->lock));

    strcat(kbuff, "\0");

    /* Cargamos los bytes leídos */
    nr_bytes=strlen(kbuff);

    /* Enviamos datos al espacio de ususario */
    if (copy_to_user(buf, kbuff, nr_bytes))
        return -EINVAL;

    (*off)+=len;  /* Update the file pointer */

    /* Informamos */
    printk(KERN_INFO "Multilist: Elements listed\n");

    return nr_bytes;
}

/* Función write */
static ssize_t listcontrol_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
   
    int available_space = BUF_LEN-1;
    char kbuff[BUF_LEN];         //Copia de buffer de usuario
    char name[100];
    int ret;

    if (len > available_space) {
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
        if(addList(name)){
            return -ENOMEM;
        }

        /* Informamos */
        printk(KERN_INFO "multilist: Added list called %s\n", name);

    } else if(sscanf(&kbuff[0],"delete %s", name)) {
        
        /* ELiminamos la lista de la multilista */
        if((ret = removeList(name, TRUE)) != TRUE){
            return ret;
        }   

        /* Informamos */
        printk(KERN_INFO "multilist: Removed list called %s\n", name);

    } else {
        return -EINVAL;
    }

    return len;
}


/*########## FUNCIONES DE GESTION DE LAS LISTAS ENLAZADAS ##########*/

/* Funcion para añadir una lista a la multilista */
int addList(char * name) {

    multilist_item_t *nodo;
    int len = 100;
    
    nodo = (multilist_item_t *)vmalloc(sizeof(multilist_item_t));
    nodo->name = (char *)vmalloc(len * sizeof(char));

    strcpy(nodo->name, name);
    
    spin_lock_init(&(nodo->lock));

    /* Inicializamos la lista */
    INIT_LIST_HEAD(nodo->mylist);
    nodo->listCount = 0;

    if(down_interruptible(&multilist_lock)){
        return -ENAVAIL;
    }

    /* Añadimos la nueva lista a la multilista */
    list_add_tail(&(nodo->links), &multilist);
    multilistCount++;

    up(&multilist_lock);

    /* Creamos la entrada de proc */
    proc_entry = proc_create_data( name, 0666, proc_dir, &proc_entry_fops, (void *) nodo);

    /* En caso de error al crear directorio /proc, eliminamos la lista añadida */
    if (proc_entry == NULL) {
        removeList(name, FALSE);
        printk(KERN_INFO "multilist: Can't create /proc entry\n");
        return -ENOMEM;
    }

    return 0;
}

/* Funcion para eliminar una lista de la multilista (y directorio /proc) */
int removeList(char *name, int remproc) {
    struct list_head* cur_node=NULL;
    struct list_head* cur_node2=NULL;
    struct list_head* aux=NULL;
    multilist_item_t *mynodo;
    list_item_t *mylistnode;
    int removed = -ENOENT;

    if(down_interruptible(&multilist_lock)){
        return -ENAVAIL;
    }

    /* Para cada nodo de la lista comprobamos si contiene num */
    list_for_each_safe(cur_node, aux, &multilist) {
        
        mynodo = list_entry(cur_node,multilist_item_t, links);

        /* Si en contramos la lista que buscamos, liberamos recursos */
        if(strcmp(mynodo->name, name) == 0) {
			
			
			/* Bloqueamos la lista */
			spin_lock(&(mynodo->lock));
			
			/* Eliminamos todos los nodos (elementos) de la lista */
			list_for_each_safe(cur_node2, aux, mynodo->mylist) {
				
				mylistnode = list_entry(cur_node2,list_item_t, links);
				
				/* Eliminamos nodo de la lista */
				list_del(cur_node2);
				
				/*Liberamos memoria dinámica del nodo */
				vfree(mylistnode);
			}		
			
			mynodo->listCount = 0;
			
			/* Desbloqueamos la lista */
			spin_unlock(&(mynodo->lock));
			
			
            /* Eliminamos nodo inicial la lista */
            list_del(mynodo->mylist);
            
            /* Eliminamos nodo de la multilista */
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

    /* Eliminamos directorio /proc correspondiente a la lista */
    if(removed && remproc)
        remove_proc_entry(name, proc_dir);

    return removed;
}

/* Funcion para eliminar todas las listas de la multilista (y /proc) */
void remove_all_list(void){
    struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;
    multilist_item_t *mynodo;

    if(down_interruptible(&multilist_lock)){
        return;
    }

    
    list_for_each_safe(cur_node, aux, &multilist) {
        
        mynodo = list_entry(cur_node,multilist_item_t, links);
              
        /* Eliminamos los elementos de la lista */      
		cleanUpList(mynodo);
		
        /* Eliminamos la lista */
        list_del(mynodo->mylist);
        
        /* Eliminamos nodo de la lista */
        list_del(cur_node);
        
        /* Eliminamos entrada proc */
        remove_proc_entry(mynodo->name, proc_dir);
        
        /* Liberamos memoria dinámica */
        vfree(mynodo->name);
        vfree(mynodo);
        multilistCount--;
    }

    up(&multilist_lock);

    return;
}
/*#################################################################*/



/*########## FUNCIONES DE GESTION DE LOS ELEMENTOS DE LAS LISTAS ##########*/

/* Funcion de comparacion */
int cmpElement(const void *a, const void *b) {
    /* Recogemos y tipamos los nodos */
    list_item_t *aa = *((list_item_t **) a);
    list_item_t *bb = *((list_item_t **) b);

    /* Devolvemos <0 si a > b o >=0 si a <= b */
    return aa->data - bb->data;
}

/* Funcion de cambio. Recibe las direcciones de dos nodos e intercambia sus contenidos */
void swapp(void * a, void * b, int size) {
    /* Recogemos y tipamos los nodos */
    list_item_t *aa = *((list_item_t **) a);
    list_item_t *bb = *((list_item_t **) b);

    /* Intercambiamos el dato de las posiciones */
    int aux = aa->data;
    aa->data = bb->data;
    bb->data = aux;
}

/* Funcion que ordena la lista */
void sortlist(multilist_item_t *lista) {   

    list_item_t *arr[lista->listCount];    // Array de punteros que guarda las
                                    //direcciones de los nodos de la lista
    int i = 0;
    list_item_t *mynodo;            // Puntero hacia el nodo actual
    struct list_head* cur_node=NULL;

	spin_lock(&(lista->lock)); 

    /* Recorremos la lista */
    list_for_each(cur_node, lista->mylist) {
        mynodo = list_entry(cur_node,list_item_t, links);

        /* Guardamos la dirección del nodo en el array */
        arr[i] = mynodo;
        i++;
    }

    /* Ordenamos la lista */
    sort(&arr[0], lista->listCount, sizeof(list_item_t *), cmpElement, swapp);

    spin_unlock(&(lista->lock));
}

/* Función que añade un elemento a la lista */
void addElement(int num, multilist_item_t *lista) {
    list_item_t *mynodo;
    
    /* Reservamos memoria dinámica para el nuevo nodo */
    mynodo = (list_item_t*) vmalloc(sizeof(list_item_t));

    spin_lock(&(lista->lock));
    /* Guardamos el valor leido */
    mynodo->data = num;

    /* Añadimos el nodo a la lista */
    list_add_tail(&(mynodo->links), lista->mylist);
    /** Incrementamos el contador de nodos */
    lista->listCount++;
    spin_unlock(&(lista->lock));   
}

/* Funcion para eliminar elemento de la lista */
void removeElement(int num, multilist_item_t *lista) {
    struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;
    list_item_t *mynodo;


    spin_lock(&(lista->lock));

    /* Para cada nodo de la lista comprobamos si contiene num */
    list_for_each_safe(cur_node, aux, lista->mylist) {
        mynodo = list_entry(cur_node,list_item_t, links);
        if(mynodo->data == num) {
            /* Eliminamos de la lista */
            list_del(cur_node);
            /* Liberamos memoria dinámica */
            vfree(mynodo);
            lista->listCount--;
        }
    }

    spin_unlock(&(lista->lock));
}

/* Función que limpia la lista entera */
void cleanUpList(multilist_item_t *lista) {
    list_item_t *mynodo;
    struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;

    spin_lock(&(lista->lock));

    /* Recorremos la lista */
    list_for_each_safe(cur_node, aux, lista->mylist) {
        mynodo = list_entry(cur_node,list_item_t, links);

        /* Eliminamos nodo de la lista */
        list_del(cur_node);

        /*Liberamos memoria dinámica del nodo */
        vfree(mynodo);
    }
    lista->listCount = 0;

    spin_unlock(&(lista->lock));
}
/*#######################################################################*/




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
    if (proc_control == NULL) {
		remove_proc_entry("listmulti", NULL);
        printk(KERN_INFO "listmulti: Can't create /proc entry\n");
        return -ENOMEM;
    }

    /* Inicializaciones */
	INIT_LIST_HEAD(&multilist);
    multilistCount = 0;
    sema_init(&multilist_lock, 1);
    /* Añadir nodo default a la lista */
    if((ret = addList("default")))
        return ret;

	printk(KERN_INFO "listmulti: Module loaded\n");

    return 0;
}

/* Función exit module */
void exit_listproc_module( void )
{
	remove_all_list();
    /* Eliminamos la entrada de proc */
	remove_proc_entry("control", proc_dir);
    remove_proc_entry("listmulti", NULL);	//Directorio

    /* Informamos */
    printk(KERN_INFO "listproc: Module unloaded.\n");
}


module_init( init_listproc_module );
module_exit( exit_listproc_module );
