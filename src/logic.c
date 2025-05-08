#include <linux/module.h>
#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>

#define BIT_1_TIME 58000  /* 58 microseconds*/
#define BIT_0_TIME 100000 /* 100 microsecdons*/

#define STACK_SIZE 4096
#define LPT1 0x378 /*Pin of parallelport*/

SEM bit_mutex;

RT_TASK msg_periodic_task;

uint64_t message = 0xFFFC066230C00000; // 0x5555555555555555;
int length = 42;
uint64_t[] msg_queue = {0xFFFC066230C00000};
int msg_count = 1;

static void send_msg_task(long n)
{
  int i = 0;
  while (1)
  {
    if (i > 0 && i < msg_count)
    {
      // TODO: buildTelegram() to create message from data object
      send_bit_task(message, length);
    }
    rt_task_wait_period();
    if (i >= msg_count)
    {
      i = 0;
    }
    else
    {
      i++;
    }
  }
}

static void send_bit_task(uint64_t message, int length)
{

  int i;
  rt_mutex_lock(&bit_mutex);
  for (i = 0; i < length; i++)
  {
    if (((message >> (63 - i)) & 0x01) == 1)
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
  rt_mutex_unlock(&bit_mutex);
}

static __init int send_init(void)
{
  rt_mount();

  rt_mutex_init(&bit_mutex);
  rt_task_init(&msg_periodic_task, send_msg_task, 0, STACK_SIZE, 1, 0, 0);

  rt_set_periodic_mode();
  start_rt_timer(nano2count(20000000));

  rt_task_make_periodic(&msg_periodic_task, rt_get_time() + nano2count(1000000000), nano2count(20000000));

  rt_printk("Module loaded\n");

  return 0;
}

static __exit void send_exit(void)
{
  stop_rt_timer();

  rt_task_delete(&msg_task);
  rt_mutex_delete(&bit_mutex)

      rt_umount();
  rt_printk("Unloading module\n");
}

module_init(send_init);
module_exit(send_exit);

MODULE_LICENSE("GPL");
