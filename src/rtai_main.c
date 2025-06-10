#include <linux/module.h>
#include <rtai_fifos.h>
#include "communication/railroad_communication.h"
#include "communication/rtai_linux_communication.h"
#define STACK_SIZE 4096
#define FIFO_SIZE 1024
#define PERIOD_TIMER 20000000
#define PERIOD_MAG_TASK 70000000
#define PERIOD_LOC_TASK 60000000

static __init int send_init(void)
{
  rt_mount();

  rt_sem_init(&bit_sem, 1);
  int i;
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_sem_init(&loc_sem[i], 1);
  }
  for (i = 0; i < MAG_MSQ_SIZE; i++)
  {
    rt_sem_init(&mag_sem[i], 1);
  }

  rtf_create(FIFO_CMD, FIFO_SIZE);
  rtf_create_handler(FIFO_CMD, &fifo_handler);
  rtf_create(FIFO_ACK, FIFO_SIZE);

  rt_task_init(magnetic_task, send_magnetic_msg_task, 0, STACK_SIZE, 2, 0, 0);
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_task_init(&loco_tasks[i], send_loco_msg_task, i, STACK_SIZE, 1, 0, 0);
  }

  rt_set_periodic_mode();
  start_rt_timer(nano2count(PERIOD_TIMER));

  rt_task_make_periodic(magnetic_task, rt_get_time() + nano2count(1000000000), nano2count(PERIOD_MAG_TASK));
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_task_make_periodic(&loco_tasks[i], rt_get_time() + nano2count(1000000000), nano2count(PERIOD_LOC_TASK + i));
  }

  rt_printk("Module loaded\n");

  return 0;
}

static __exit void send_exit(void)
{
  stop_rt_timer();

  rt_task_delete(magnetic_task);
  int i;
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_task_delete(&loco_tasks[i]);
  }

    rtf_destroy(FIFO_CMD);
  rtf_destroy(FIFO_ACK);

  rt_sem_delete(&bit_sem);
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_sem_delete(&loc_sem[i]);
  }
  for (i = 0; i < MAG_MSQ_SIZE; i++)
  {
    rt_sem_delete(&mag_sem[i]);
  }

  rt_umount();
  rt_printk("Unloading module\n");
}

module_init(send_init);
module_exit(send_exit);

MODULE_LICENSE("GPL");