#include <linux/module.h>
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/proc_fs.h>

#define BUF_LEN 100

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ledctl");
MODULE_AUTHOR("Miguel Higuera Romero & Alejandro Nicolás Ibarra Loik");

struct tty_driver* kbd_driver= NULL;
static struct proc_dir_entry *proc_entry;

/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set led state to that specified by mask */
inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

void ledctl_exit(void){
    remove_proc_entry("ledctl", NULL);
    printk(KERN_INFO "Ledctl: Module unloaded\n");
}

static ssize_t ledctl_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    char kbuf[BUF_LEN];
    int mask;

    if ((*off) > 0) /* Tell the application that there is nothing left to read */
      return 0;

    if(len > BUF_LEN - 1)
      return -ENOSPC;

    if (copy_from_user( &kbuf[0], buf, len ))
      return -EFAULT;

    kbuf[len] = '\0';
    *off += len;

    if(sscanf(&kbuf[0], "%i", &mask)){
      if(mask >= 0 && mask <= 7){
	set_leds(kbd_driver, (unsigned int) mask);
      }else{
	return -EINVAL;
      }
    }else{
      return -EINVAL;
    }
    return len;
}

static ssize_t ledctl_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  char kbuff[BUF_LEN];
  kbuff[0] = '\0';
  strcat(&kbuff[0], "Nothing to read");
  if(copy_to_user(&kbuff[0], buf, strlen(kbuff))){
    return -EFAULT;
  }
  return strlen(kbuff);
}

static const struct file_operations proc_entry_fops = {
    .read = ledctl_read,
    .write = ledctl_write,
};

int ledctl_init(void)
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
