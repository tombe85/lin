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
MODULE_DESCRIPTION("Productor/Consumidor v2 para DSO");
MODULE_AUTHOR("Juan Carlos Sáez");

static struct proc_dir_entry *proc_entry;
static cbuffer_t* cbuf;
struct semaphore prod_queue,cons_queue;
struct semaphore mtx;
int nr_prod_waiting,nr_cons_waiting;

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

  /* Acceso a la sección crítica */
  if (down_interruptible(&mtx))
  {
	return -EINTR;
  }

  /* Bloquearse mientras no haya huecos en el buffer */
  while (is_full_cbuffer_t(cbuf))
  {
	/* Incremento de productores esperando */
	nr_prod_waiting++;

	/* Liberar el 'mutex' antes de bloqueo*/
	up(&mtx);

	/* Bloqueo en cola de espera */		
	if (down_interruptible(&prod_queue)){
		down(&mtx);
		nr_prod_waiting--;
		up(&mtx);		
		return -EINTR;
	}	

	/* Readquisición del 'mutex' antes de entrar a la SC */				
	if (down_interruptible(&mtx)){
		return -EINTR;
	}	
  }

  /* Insertar en el buffer */
  insert_cbuffer_t(cbuf,item); 
  
  /* Despertar a los productores bloqueados (si hay alguno) */
  if (nr_cons_waiting>0)
  {
	up(&cons_queue);	
	nr_cons_waiting--;
  }

  /* Salir de la sección crítica */
  up(&mtx);
  
  return len;
}


static ssize_t prodcons_read(struct file *filp, char __user *buf, size_t len, loff_t *off) 
{
  int nr_bytes=0;
  int* item=NULL;
  char kbuff[32]="";
  
  if ((*off) > 0) 
      return 0;

  /* Entrar a la sección crítica */
  if (down_interruptible(&mtx))
  {
	return -EINTR;
  }

 /* Bloquearse mientras buffer esté vacío */
  while (size_cbuffer_t(cbuf)==0)
  {
	/* Incremento de consumidores esperando */
	nr_cons_waiting++;

	/* Liberar el 'mutex' antes de bloqueo*/
	up(&mtx);
	
	/* Bloqueo en cola de espera */		
	if (down_interruptible(&cons_queue)){
		down(&mtx);
		nr_cons_waiting--;
		up(&mtx);		
		return -EINTR;
	}	
	
	/* Readquisición del 'mutex' antes de entrar a la SC */		
	if (down_interruptible(&mtx)){
		return -EINTR;
	}	
  }

  /* Obtener el primer elemento del buffer y eliminarlo */
  item=head_cbuffer_t(cbuf);
  remove_cbuffer_t(cbuf);  
  
  /* Despertar a los consumidores bloqueados (si hay alguno) */
  if (nr_prod_waiting>0)
  {
	up(&prod_queue);	
	nr_prod_waiting--;
  }

  /* Salir de la sección crítica */	
  up(&mtx);
   
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

  /* Inicialización a 0 de los semáforos usados como colas de espera */
  sema_init(&prod_queue,0);
  sema_init(&cons_queue,0);

  /* Inicializacion a 1 del semáforo que permite acceso en exclusión mutua a la SC */
  sema_init(&mtx,1);

  nr_prod_waiting=nr_cons_waiting=0;

  proc_entry = proc_create_data("prodcons",0666, NULL, &proc_entry_fops, NULL);
    
  if (proc_entry == NULL) {
      destroy_cbuffer_t(cbuf);
      printk(KERN_INFO "Prodcons2: No puedo crear la entrada en proc\n");
      return  -ENOMEM;
  }
      
  printk(KERN_INFO "Prodcons2: Cargado el Modulo.\n");
  
  return 0;
}


void exit_prodcons_module( void )
{
  remove_proc_entry("prodcons", NULL);
  destroy_cbuffer_t(cbuf);
  printk(KERN_INFO "Prodcons2: Modulo descargado.\n");
}


module_init( init_prodcons_module );
module_exit( exit_prodcons_module );
