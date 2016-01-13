#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

/* Workqueue descriptor */
static struct workqueue_struct* my_wq;  

/* Work descriptor */
struct work_struct my_work;

/* Work's handler function */
static void my_wq_function( struct work_struct *work )
{
    printk(KERN_INFO "HELLO WORLD!!\n");
}

int init_module( void )
{
  /* Create a private workqueue named 'my_queue' */
  my_wq = create_workqueue("my_queue");
  
  if (!my_wq)
    return -ENOMEM;

  /* Initialize work structure (with function) */
  INIT_WORK(&my_work, my_wq_function );

  /* Enqueue work */
  queue_work( my_wq, &my_work );

  return 0;
}

void cleanup_module( void )
{
  /* Wait until all jobs scheduled so far have finished */
  flush_workqueue( my_wq );
  
  /* Destroy workqueue resources */
  destroy_workqueue( my_wq );
}
