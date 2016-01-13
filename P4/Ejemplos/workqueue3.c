#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

/* Workqueue descriptor */
static struct workqueue_struct *my_wq;  

/* Wrapper for the work_struct structure,
  that makes it possible to pass parameters more easily
  to the work's handler function */
typedef struct {
  struct work_struct my_work;
  int    x;
} my_work_t;

/* Descriptor of the two pieces of deferrable work */
my_work_t *work, *work2;


/* Work's handler function */
static void my_wq_function( struct work_struct *work )
{
  my_work_t *my_work = (my_work_t *)work;

  printk(KERN_INFO "my_work.x %d\n", my_work->x );

  kfree( (void *)work );
}

int init_module( void )
{
  int ret=0;
    
  /* Create a private workqueue named 'my_queue' */
  my_wq = create_workqueue("my_queue");
  
  if (my_wq) {

    /* Queue some work (item 1) */
  
    /* Allocate memory */
    work = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    if (work) {

      /* Initialize work structure (with function) */
      INIT_WORK( (struct work_struct *)work, my_wq_function );

      /* Set up a value for the parameter */
      work->x = 1;

      /* Enqueue work */
      ret = queue_work( my_wq, (struct work_struct *)work );
    }

    /* Queue some additional work (item 2) */
    
    /* Allocate memory */
    work2 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    if (work2) {

      /* Initialize work structure (with function) */
      INIT_WORK( (struct work_struct *)work2, my_wq_function );

      /* Set up a value for the parameter */
      work2->x = 2;
  
      /* Enqueue work */
      ret = queue_work( my_wq, (struct work_struct *)work2 );
    }

  }
  return 0;
}

void cleanup_module( void )
{
  /* Wait until all jobs scheduled so far have finished */
  flush_workqueue( my_wq );
  
  /* Destroy workqueue resources */
  destroy_workqueue( my_wq );
}
