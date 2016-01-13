#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/timer.h>


struct timer_list my_timer; /* Structure that describes the kernel timer */


/* Function invoked when timer expires (fires) */
static void fire_timer(unsigned long data)
{
	static char flag=0;
        
        if (flag==0)
            printk(KERN_INFO "Tic\n");
        else
            printk(KERN_INFO "Tac\n");           
	
        flag=~flag;
        
        /* Re-activate the timer one second from now */
	mod_timer( &(my_timer), jiffies + HZ); 
}

int init_timer_module( void )
{
    /* Create timer */
    init_timer(&my_timer);
    /* Initialize field */
    my_timer.data=0;
    my_timer.function=fire_timer;
    my_timer.expires=jiffies + HZ;  /* Activate it one second from now */
    /* Activate the timer for the first time */
    add_timer(&my_timer); 
    return 0;
}


void cleanup_timer_module( void ){
  /* Wait until completion of the timer function (if it's currently running) and delete timer */
  del_timer_sync(&my_timer);
}

module_init( init_timer_module );
module_exit( cleanup_timer_module );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("timermod Module");
MODULE_AUTHOR("Juan Carlos Saez");
