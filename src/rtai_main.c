#include <linux/module.h>
#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>
#include <rtai_fifos.h>

#include "communication/railroad_communication.h"
#include "communication/rtai_linux_communication.h"

#define STACK_SIZE 4096
#define PERIOD_TIMER 1000000
#define PERIOD_MAG_TASK 50000000
#define PERIOD_LOC_TASK 5000000

static __init int send_init(void)
{
  rt_printk("Start loading module...");
  int i;
  int ret = 0;
  int task_make_res;
  int fifo_create_res;
  int task_init_res;
  rt_mount();

  rt_printk("Initialize sem");
  rt_sem_init(&bit_sem, 1);
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_sem_init(&loc_sem[i], 1);
  }
  for (i = 0; i < MAG_MSQ_SIZE; i++)
  {
    rt_sem_init(&mag_sem[i], 1);
  }

  rt_printk("Create FIFO Channel %d with size %d", FIFO_CMD, FIFO_SIZE);
  fifo_create_res = rtf_create(FIFO_CMD, FIFO_SIZE);
  if (fifo_create_res != 0)
  {
    rt_printk("Failed to create cmd fifo (channel %d) with %d!\n", FIFO_CMD, fifo_create_res);
    ret = -1;
  }
  rt_printk("Create FIFO handler for Channel %d", FIFO_CMD);
  int handler_res = rtf_create_handler(FIFO_CMD, &fifo_handler);
  if (handler_res != 0)
  {
    rt_printk("Failed to create cmd fifo handler with %d!\n", handler_res);
    ret = -1;
  }

  rt_printk("Create FIFO Channel %d with size %d", FIFO_ACK, FIFO_SIZE);
  fifo_create_res = rtf_create(FIFO_ACK, FIFO_SIZE);
  if (fifo_create_res != 0)
  {
    rt_printk("Failed to create ack fifo (channel %d) with %d!\n", FIFO_ACK, fifo_create_res);
    ret = -1;
  }

  task_init_res = rt_task_init(&init_task, send_init_dcc, 0, STACK_SIZE, 1, 0, 0);
  if (task_init_res != 0)
  {
    rt_printk("Failed to init 'inti' task with %d!\n", task_init_res);
    ret = -1;
  }
  // task_init_res = rt_task_init(&magnetic_task, send_magnetic_msg_task, 0, STACK_SIZE, 3, 0, 0);
  // if (task_init_res != 0)
  // {
  //   rt_printk("Failed to init 'magnetic' task with %d!\n", task_init_res);
  //   ret = -1;
  // }
  // Initializes for each locomotive its own periodic task with
  // the task pointer, the function, the index of the current loc which specify for which loc the task is responsible for, the Stack size, priority
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    task_init_res = rt_task_init(&loco_tasks[i], send_loco_msg_task, i, STACK_SIZE, 2, 0, 0);
    if (task_init_res != 0)
    {
      rt_printk("Failed to init 'locomotive %d' task with %d!\n", i, task_init_res);
      ret = -1;
    }
  }

  rt_set_oneshot_mode();
  start_rt_timer(nano2count(PERIOD_TIMER));

  rt_printk("Make one shot task 'init'\n");
  task_make_res = rt_task_make_periodic(&init_task, rt_get_time() + nano2count(10000000), 0);
  if (task_make_res != 0)
  {
    rt_printk("Failed to make one shot init task with %d!\n", task_make_res);
    ret = -1;
  }
  // rt_printk("Make periodic task 'mag'\n");
  // task_make_res = rt_task_make_periodic(&magnetic_task, rt_get_time() + nano2count(10000000), nano2count(PERIOD_MAG_TASK));
  // if (task_make_res != 0)
  // {
  //   rt_printk("Failed to make periodic magnetic task with %d!\n", task_make_res);
  //   ret = -1;
  // }
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_printk("Make periodic task 'loc %d'\n", i);
    task_make_res = rt_task_make_periodic(&loco_tasks[i], rt_get_time() + nano2count(10000000), nano2count(PERIOD_LOC_TASK + i));
    if (task_make_res != 0)
    {
      rt_printk("Failed to make periodic locomotive %d task with %d!\n", i, task_make_res);
      ret = -1;
    }
  }

  rt_printk("Module loaded\n");

  return ret;
}

static __exit void send_exit(void)
{
  rt_printk("Start unloading module...");
  int i;

  stop_rt_timer();

  rt_task_delete(&init_task);
  // rt_task_delete(&magnetic_task);
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    rt_task_delete(&loco_tasks[i]);
  }

  rtf_destroy(FIFO_ACK);
  rtf_destroy(FIFO_CMD);

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