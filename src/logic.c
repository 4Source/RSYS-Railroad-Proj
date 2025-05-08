#include <linux/module.h>
#include <rtai.h>
#include <rtai_sched.h>

#define BIT_1_TIME 58000  /* 58 microseconds*/
#define BIT_0_TIME 100000 /* 100 microsecdons*/

#define STACK_SIZE 4096
#define LPT1 0x378 /*Pin des Parallelports*/

RT_TASK mytask;

uint64_t message = 0x5555555555555555;
int length = 42;

static void send_bit_task(/*uint64_t message, int length*/)
{
  int i;
  for (i = 0; i < length; i++)
  {
    if ((message >> (63 - i)) & 0x01 == 1)
    {
      outb(0x00, LPT1);
      rt_sleep(nano2count(BIT_1_TIME));
      outb(0xFF, LPT1);
      rt_sleep(nano2count(BIT_1_TIME));
    }
    else
    {
      outb(0x00, LPT1);
      rt_sleep(nano2count(BIT_0_TIME));
      outb(0xFF, LPT1);
      rt_sleep(nano2count(BIT_0_TIME));
    }
  }
}

static __init int send_init(void)
{
  rt_mount();

  rt_task_init(&mytask, send_bit_task, 0, STACK_SIZE, 1, 0, 0);

  rt_set_oneshot_mode();
  start_rt_timer(0);

  rt_task_resume(&mytask);

  rt_printk("Module loaded\n");

  return 0;
}

static __exit void send_exit(void)
{
  stop_rt_timer();

  rt_task_delete(&mytask);

  rt_umount();
  rt_printk("Unloading module\n");
}

module_init(send_init);
module_exit(send_exit);

MODULE_LICENSE("GPL");
