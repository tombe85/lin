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
static char *modlist;  // Space for the "modlist"

struct list_head mylist; /* Lista enlazada */
/* Nodos de la lista */
typedef struct {
  int data;
  struct list_head links;
} list_item_t;





static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
  int available_space = BUFFER_LENGTH-1;

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
  int ret = 0;

  /* Recervamos memoria dinamica */
  modlist = (char *)vmalloc( BUFFER_LENGTH );

  if (!modlist) {
    ret = -ENOMEM;
  } else {

    /* Asignamos la variable en memoria dinamica como variable compartida */
    memset( modlist, 0, BUFFER_LENGTH );
    proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops);
    if (proc_entry == NULL) {
      ret = -ENOMEM;
      vfree(modlist);
      printk(KERN_INFO "modlist: Can't create /proc entry\n");
    } else {
      printk(KERN_INFO "modlist: Module loaded\n");
    }
  }

  return ret;

}


void exit_modlist_module( void )
{
  remove_proc_entry("modlist", NULL);
  vfree(modlist);
  printk(KERN_INFO "modlist: Module unloaded.\n");
}


module_init( init_modlist_module );
module_exit( exit_modlist_module );
