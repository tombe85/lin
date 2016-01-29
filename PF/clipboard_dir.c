#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Clipboard Kernel Module - FDI-UCM");

#define BUFFER_LENGTH       PAGE_SIZE

static struct proc_dir_entry *proc_entry;
static char *clipboard;  // Space for the "clipboard"
struct proc_dir_entry *test_dir=NULL;

static ssize_t clipboard_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    int available_space = BUFFER_LENGTH-1;

    if ((*off) > 0) /* The application can write in this entry just once !! */
        return 0;

    if (len > available_space) {
        printk(KERN_INFO "clipboard: not enough space!!\n");
        return -ENOSPC;
    }

    /* Transfer data from user to kernel space */
    if (copy_from_user( &clipboard[0], buf, len ))
        return -EFAULT;

    clipboard[len] = '\0'; /* Add the `\0' */
    *off+=len;            /* Update the file pointer */

    return len;
}

static ssize_t clipboard_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{

    int nr_bytes;

    if ((*off) > 0) /* Tell the application that there is nothing left to read */
        return 0;

    nr_bytes=strlen(clipboard);

    if (len<nr_bytes)
        return -ENOSPC;

    /* Transfer data from the kernel to userspace */
    if (copy_to_user(buf, clipboard,nr_bytes))
        return -EINVAL;

    (*off)+=len;  /* Update the file pointer */

    return nr_bytes;
}

static const struct file_operations proc_entry_fops = {
    .read = clipboard_read,
    .write = clipboard_write,
};



int init_clipboard_module( void )
{
    clipboard = (char *)vmalloc( BUFFER_LENGTH );

    if (!clipboard) {
        return  -ENOMEM;
    }

    memset( clipboard, 0, BUFFER_LENGTH );

    /* Create proc directory */
    test_dir=proc_mkdir("test",NULL);

    if (!test_dir) {
        vfree(clipboard);
        return -ENOMEM;
    }

    /* Create proc entry /proc/test/clipboard */
    proc_entry = proc_create( "clipboard", 0666, test_dir, &proc_entry_fops);

    if (proc_entry == NULL) {
        remove_proc_entry("test", NULL);
        vfree(clipboard);
        return -ENOMEM;
    }

    printk(KERN_INFO "Clipboard: Module loaded\n");

    return 0;
}


void exit_clipboard_module( void )
{
    remove_proc_entry("clipboard", test_dir);
    remove_proc_entry("test", NULL);
    vfree(clipboard);
    printk(KERN_INFO "Clipboard: Module removed.\n");
}


module_init( init_clipboard_module );
module_exit( exit_clipboard_module );
