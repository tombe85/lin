#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <asm-generic/errno.h>
#include <linux/semaphore.h>
#include "cbuffer.h"


#define MAX_ITEMS_CBUF	5
#define MAX_CHARS_KBUF	10

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Productor/Consumidor v1.1 para DSO");
MODULE_AUTHOR("Juan Carlos Sáez");

static struct proc_dir_entry *proc_entry;
static cbuffer_t* cbuf; /* Buffer circular compartido */
struct semaphore elementos,huecos; /* Semaforos para productor y consumidor */
struct semaphore mtx; /* Para garantizar exclusión mutua en acceso a buffer */

static ssize_t prodcons_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
 
  char kbuf[MAX_CHARS_KBUF+1];
  int val=0;
  int* item=NULL;
  
  if ((*off) > 0) /* The application can write in this entry just once !! */
    return 0;

  if (len > MAX_CHARS_KBUF) {
    return -ENOSPC;
  }
  if (copy_from_user( kbuf, buf, len )) {
    return -EFAULT;
  }

  kbuf[len] ='\0'; 
  *off+=len;            /* Update the file pointer */
  
  if (sscanf(kbuf,"%i",&val)!=1)
  {
	return -EINVAL;
  }
	
  item=vmalloc(sizeof(int));

  (*item)=val;

  /* Bloqueo hasta que haya huecos */  
  if (down_interruptible(&huecos))
  {
	vfree(item);
	return -EINTR;
  }

  /* Entrar a la SC */                              
  if (down_interruptible(&mtx))
  {
        up(&huecos);      
        return -EINTR;
  }

  /* Inserción segura en el buffer circular */
  insert_cbuffer_t(cbuf,item);

  /* Salir de la SC */
  up(&mtx);
 
  /* Incremento del número de elementos (reflejado en el semáforo) */
  up(&elementos);
  
  return len;
}


static ssize_t prodcons_read(struct file *filp, char __user *buf, size_t len, loff_t *off) 
{

  int nr_bytes=0;
  int* item=NULL;
  char kbuff[32]="";

  if ((*off) > 0) 
      return 0;

  /* Bloqueo hasta que haya elementos que consumir */  
  if (down_interruptible(&elementos))
  {
	return -EINTR;
  }

  /* Entrar a la SC  */  
  if (down_interruptible(&mtx))
  {
 	up(&elementos);	
	return -EINTR;
  }

  /* Obtener el primer elemento del buffer y eliminarlo */
  item=head_cbuffer_t(cbuf);
  remove_cbuffer_t(cbuf);  

  /* Salir de la SC */ 
  up(&mtx);
 
  /* Incremento del número de huecos (reflejado en el semáforo) */
  up(&huecos);
   
  /* Conversion a cadena de caracteres para el usuario */
  nr_bytes=sprintf(kbuff,"%i\n",*item);
  
  /* Liberar memoria del elemento extraido */
  vfree(item);
  
  if (len < nr_bytes)
    return -ENOSPC;
  
  if (copy_to_user(buf,kbuff,nr_bytes))
    return -EINVAL;
   
  (*off)+=nr_bytes;  /* Update the file pointer */

  return nr_bytes;
}

static const struct file_operations proc_entry_fops = {
    .read = prodcons_read,
    .write = prodcons_write,    
};


int init_prodcons_module( void )
{

  /* Inicialización del buffer */  
  cbuf = create_cbuffer_t(MAX_ITEMS_CBUF);

  if (!cbuf) {
    return -ENOMEM;
  }
  
  /* Semaforo elementos inicializado a 0 (buffer vacío) */
  sema_init(&elementos,0); 

  /* Semaforo huecos inicializado a MAX_ITEMS_CBUF (buffer vacío) */
  sema_init(&huecos,MAX_ITEMS_CBUF);

  /* Semaforo para garantizar exclusion mutua */
  sema_init(&mtx,1);
  
  proc_entry = proc_create_data("prodcons",0666, NULL, &proc_entry_fops, NULL);
    
  if (proc_entry == NULL) {
      destroy_cbuffer_t(cbuf);
      printk(KERN_INFO "Prodcons1: No puedo crear la entrada en proc\n");
      return  -ENOMEM;
  }
      
  printk(KERN_INFO "Prodcons1: Cargado el Modulo.\n");
  
  return 0;
}


void exit_prodcons_module( void )
{
  remove_proc_entry("prodcons", NULL);
  destroy_cbuffer_t(cbuf);
  printk(KERN_INFO "Prodcons1: Modulo descargado.\n");
}


module_init( init_prodcons_module );
module_exit( exit_prodcons_module );
