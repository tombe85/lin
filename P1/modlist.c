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

#define BUFFER_LENGTH       PAGE_SIZE

static struct proc_dir_entry *proc_entry;
static char *modlist;  // Space for the "modlist" buffer

struct list_head mylist; /* Lista enlazada */
/* Nodos de la lista */
typedef struct {
  int data;
  struct list_head links;
} list_item_t;


void init_mylist(void) {
  INIT_LIST_HEAD(&mylist);
}

static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
  int available_space = BUFFER_LENGTH-1;
  int num;
  list_item_t *mynodo;
  
  if ((*off) > 0) /* The application can write in this entry just once !! */
    return 0;

  if (len > available_space) {
    printk(KERN_INFO "modlist: not enough space!!\n");
    return -ENOSPC;
  }

  /* Transfer data from user to kernel space */
  if (copy_from_user( &modlist[0], buf, len ))
    return -EFAULT;

  modlist[len] = '\0'; /* Add the `\0' */
  *off+=len;            /* Update the file pointer */

  if(sscanf(&modlist[0],"add %i",&num)) {
    //printk(KERN_INFO "Se ha leido add %i\n",num);

    /* Creamos el nuevo nodo */
    mynodo = (list_item_t*) vmalloc(sizeof(list_item_t));
    /* Guardamos el valor leido */
    mynodo->data = num;
    /* Añadimos el nodo a la lista */
    list_add_tail(&mynodo->links, &mylist);
    printk(KERN_INFO "Añadido el elemento %i a la lista\n", num);

  }
  else if(sscanf(&modlist[0],"remove %i",&num)) {

    list_item_t *item=NULL;
    struct list_head* cur_node=NULL;
    list_for_each(cur_node, &mylist) {
      /* item points to the structure wherein the links are embedded */
      item = list_entry(cur_node,list_item_t, links);
      printk(KERN_INFO "Borrando elemento %i\n", item->data);
    }

  }
  else if(sscanf(&modlist[0],"cleanup")) {

  }
  else {
    // retornar un codigo de error correspondiente (instruccion incorrecta)
  }

  trace_printk("Current value of modlist: %s\n",modlist);

  return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

  int nr_bytes;

  if ((*off) > 0) /* Tell the application that there is nothing left to read */
      return 0;

  nr_bytes=strlen(modlist);

  if (len<nr_bytes)
    return -ENOSPC;

    /* Transfer data from the kernel to userspace */
  if (copy_to_user(buf, modlist,nr_bytes))
    return -EINVAL;

  (*off)+=len;  /* Update the file pointer */

  return nr_bytes;
}

static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,
};



int init_modlist_module( void )
{

  /* Recervamos memoria dinamica */
  modlist = (char *)vmalloc( BUFFER_LENGTH );

  if (!modlist) {
    return -ENOMEM;
  } else {

    /* Asignamos la variable en memoria dinamica como variable compartida */
    //memset( modlist, 0, BUFFER_LENGTH );
    proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops);
    if (proc_entry == NULL) {
      vfree(modlist);
      printk(KERN_INFO "modlist: Can't create /proc entry\n");
      return -ENOMEM;
    } else {
      printk(KERN_INFO "modlist: Module loaded\n");
    }

    /* Inicializamos la lista */
    init_mylist();
  }

  return 0;

}


void exit_modlist_module( void )
{
  remove_proc_entry("modlist", NULL);
  vfree(modlist);
  printk(KERN_INFO "modlist: Module unloaded.\n");
}


module_init( init_modlist_module );
module_exit( exit_modlist_module );
