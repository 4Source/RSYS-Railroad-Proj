#include <linux/module.h>
#include "communication/rtai_linux_communication.h"

#define STACK_SIZE 4096
#define FIFO_SIZE 1024

static __init int send_init(void)
{
  rt_mount();

  rtf_create(FIFO_CMD, FIFO_SIZE);
  rtf_create_handler(FIFO_CMD, &fifo_handler);
  rtf_create(FIFO_ACK, FIFO_SIZE);

  rt_mutex_init(&bit_mutex);
  rt_task_init(&msg_periodic_task, send_msg_task, 0, STACK_SIZE, 1, 0, 0);

  rt_set_periodic_mode();
  start_rt_timer(nano2count(20000000));

  for (int i = 0; i < 3; i++) {
    rt_task_init(&loco_tasks[i], send_loco_msg_task, i, STACK_SIZE, 1, 0, 0)
    rt_task_make_periodic(&loco_tasks[i], rt_get_time() + nano2count(1000000000 + i * 1000000), nano2count(60000000));
  }

  rt_task_init(&magnetic_task, rt_get_time() + nano2count(1000000000), nano2count(20000000));
  
  rt_printk("Module loaded\n");

  return 0;
}

static __exit void send_exit(void)
{
  stop_rt_timer();

  for (int i = 0; i < NUM_LOCOMOTIVES; i++) {
    rt_task_delete(&loco_tasks[i]);
  } 
  rtf_destroy(FIFO_CMD);
  rtf_destroy(FIFO_ACK);
  rt_mutex_delete(&bit_mutex);

  rt_umount();
  rt_printk("Unloading module\n");
}

module_init(send_init);
module_exit(send_exit);

MODULE_LICENSE("GPL");