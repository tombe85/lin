#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/ftrace.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modlist Kernel Module - FDI-UCM");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");

#define BUF_LEN 100
#define MAX_CHAR 20

/* Entrada de proc */
static struct proc_dir_entry *proc_entry;

/* Lista enlazada */
struct list_head mylist;

/* Nodos de la lista */
typedef struct {
  char data[MAX_CHAR];
  struct list_head links;
} list_item_t;

/* Función que limpia la lista entera */
void cleanUpList(void){
    list_item_t *mynodo;
    struct list_head* cur_node=NULL;
    struct list_head* aux=NULL;

    /* Recorremos la lista */
    list_for_each_safe(cur_node, aux, &mylist) {
        mynodo = list_entry(cur_node,list_item_t, links);
        /* Eliminamos nodo de la lista */
        list_del(cur_node);

        /*Liberamos memoria dinámica del nodo */
        vfree(mynodo);
    }
}


/* Función write */
static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
  int available_space = BUF_LEN-1;
  char elemstr[MAX_CHAR];   //Elemento de la lista
  char procstr[BUF_LEN];   //Cadena escrita en /proc
  char kbuff[BUF_LEN];  //Copia de buffer de usuario
  list_item_t *mynodo;    //nodo a añadir/eliminar

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

  *off+=len;            /* Update the file pointer */

  /* Leemos ordenes */
  if(sscanf(&kbuff[0],"add %s", elemstr)) {

    /* Creamos el nuevo nodo */
    mynodo = (list_item_t*) vmalloc(sizeof(list_item_t));

    /* Quitamos el \n de la cadena */
    kbuff[len-1] = '\0';

    /* Si la cadena supera el máximo de caracteres se limita */
    if(len - 4 > MAX_CHAR)
        kbuff[MAX_CHAR + 3] = '\0';

    /* Guardamos el valor leido de la entrada desplazada 4 bytes (longitud 'add ')*/
    strcpy(mynodo->data, &kbuff[0]+(4*sizeof(char)));

    /* Añadimos el nodo a la lista */
    list_add_tail(&mynodo->links, &mylist);

    /* Informamos */
    printk(KERN_INFO "Modlist: Added element %s to the list\n",mynodo->data);

  }
  else if(sscanf(&kbuff[0],"remove %s", elemstr)) {

    /* Quitamos el \n de la cadena */
    kbuff[len-1] = '\0';

    /* Si la lista no esta vacia */
    if(list_empty(&mylist) == 0) {
      struct list_head* cur_node=NULL;
      struct list_head* aux=NULL;
      int deletes = 0;

      list_for_each_safe(cur_node, aux, &mylist) {
        mynodo = list_entry(cur_node,list_item_t, links);
        /* Comparamos el dato almacenado con la entrada desplazada 7 bytes (numero de caracteres de 'remove ')*/
        if(strcmp(mynodo->data, &kbuff[0]+(7*sizeof(char)))== 0) {
          /* Eliminamos de la lista */
          list_del(cur_node);
          /* Liberamos memoria dinámica */
          vfree(mynodo);
          /* Mostramos info del kernel una sola vez*/
          if(deletes == 0){
            printk(KERN_INFO "Modlist: Element %s deleted from list\n", &kbuff[0]+(7*sizeof(char)));
            deletes++;
          }
        }
      }
    }else
      printk(KERN_INFO "Modlist: Can't delete elements. The list is empty\n");
  }
  else if(sscanf(&kbuff[0],"%s", procstr)){
    if(strcmp("cleanup", procstr)==0) {

      /* Limpiamos la lista entera */
      if(list_empty(&mylist) == 0) {
        cleanUpList();
        /* Informamos */
        printk(KERN_INFO "Modlist: List cleaned up\n");
      }else
        printk(KERN_INFO "Modlist: Can't cleanup list. The list is empty\n");

    }
  }
  else {
    // retornar un codigo de error correspondiente (instruccion incorrecta)
    return -EINVAL;
  }
  return len;
}

/* Función read */
static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

  int nr_bytes;           // Bytes leídos
  list_item_t *item=NULL;     //Nodo a leer
  struct list_head* cur_node=NULL;
  char kbuff[BUF_LEN];        //Mensaje final
  char tmpstr[BUF_LEN];       //Mensajes temporales
  kbuff[0] = '\0';

  if ((*off) > 0) /* Tell the application that there is nothing left to read */
      return 0;

  /* Listamos los elementos de la lista */
  list_for_each(cur_node, &mylist) {
    item = list_entry(cur_node,list_item_t, links);

    /* Escribimos en msgtmp el dato, el \n y lo concatenamos al msg final */
    sprintf(tmpstr, "%s\n", item->data);
    strcat(kbuff, tmpstr);
  }
  strcat(kbuff, "\0");

  /* Cargamos los bytes leídos */
  nr_bytes=strlen(kbuff);

  /* Enviamos datos al espacio de ususario */
  if (copy_to_user(buf, kbuff, nr_bytes))
    return -EINVAL;

  (*off)+=len;  /* Update the file pointer */

  /* Informamos */
  printk(KERN_INFO "Modlist: Elements listed\n");

  return nr_bytes;
}

static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,
};



int init_modlist_module( void )
{
  /* Creamos la entrada de proc */
  proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops);
  if (proc_entry == NULL) {
    printk(KERN_INFO "Modlist: Can't create /proc entry\n");
    return -ENOMEM;
  } else {
    printk(KERN_INFO "Modlist: Module loaded\n");
  }

  /* Inicializamos la lista */
  INIT_LIST_HEAD(&mylist);

  return 0;
}


void exit_modlist_module( void )
{
  /* Limpiamos la lista para liberar memoria */
  if(list_empty(&mylist) == 0)
    cleanUpList();

  /* Eliminamos la entrada de proc */
  remove_proc_entry("modlist", NULL);

  /* Informamos */
  printk(KERN_INFO "Modlist: Module unloaded.\n");
}


module_init( init_modlist_module );
module_exit( exit_modlist_module );
