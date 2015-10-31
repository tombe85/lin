#include <linux/module.h> 
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/proc_fs.h>

#define BUF_LEN 50

struct tty_driver* kbd_driver= NULL;
static struct proc_dir_entry *proc_entry;

/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

static void __exit ledctl_exit(void){
  printk(KERN_INFO "Ledctl: Module unloaded\n");
}

static ssize_t ledctl_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    char kbuf[BUF_LEN];
    unsigned int mask;
    
    if ((*off) > 0) /* Tell the application that there is nothing left to read */
      return 0;
      
    if(len > BUF_LEN - 1)
      return -ENOSPC;
    
    if (copy_from_user( &kbuf[0], buf, len ))
      return -EFAULT;
      
    kbuf[len] = '\0';
    *off += len;
    
    if(sscanf(kbuf, "0x%ui", &mask)){
      if(mask >= 0 && mask <= 7){
	set_leds(kbd_driver, mask);
      }else{
	return -EINVAL;
      }
    }else{
      return -EINVAL;
    }
    return len;
}

static ssize_t ledctl_read(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
  return len;
}

static const struct file_operations proc_entry_fops = {
    .write = ledctl_write,
    .read = ledctl_read,
};

static int __init ledctl_init(void)
{	
   kbd_driver= get_kbd_driver_handler();
   
   proc_entry = proc_create( "ledctl", 0666, NULL, &proc_entry_fops);
    if (proc_entry == NULL) {
      printk(KERN_INFO "Ledctl: Can't create /proc entry\n");
      return -ENOMEM;
    } else {
      printk(KERN_INFO "Ledctl: Module loaded\n");
    }
   
   return 0;
}

module_init(ledctl_init);
module_exit(ledctl_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ledctl");
