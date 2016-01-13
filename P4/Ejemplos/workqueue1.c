#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

/* Work descriptor */
struct work_struct my_work;

/* Work's handler function */
static void my_wq_function( struct work_struct *work )
{
    printk(KERN_INFO "HELLO WORLD!!\n");
}

int init_module( void )
{
  /* Initialize work structure (with function) */
  INIT_WORK(&my_work, my_wq_function );

  /* Enqueue work */
  schedule_work(&my_work);

  return 0;
}

void cleanup_module( void )
{
  /* Wait until all jobs scheduled so far have finished */
  flush_scheduled_work();
}
