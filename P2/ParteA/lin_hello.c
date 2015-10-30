#include <linux/syscalls.h> /* For SYSCALL_DEFINEi() */
#include <linux/kernel.h>

SYSCALL_DEFINE0(lin_hello)
{
    printk(KERN_DEBUG "Hello world\n");
    return 0;
}
