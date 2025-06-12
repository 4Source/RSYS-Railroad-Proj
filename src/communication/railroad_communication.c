#include "communication/railroad_communication.h"

void send_bit_task(unsigned long long message, int bit_message_length)
{
  RTIME bit_1_count = nano2count(BIT_1_TIME);
  RTIME bit_0_count = nano2count(BIT_0_TIME);

  // Acquire semaphore to access parallel port LPT1.
  rt_sem_wait(&bit_sem);
  outb(0x00, LPT1);             // Set initial voltage level
  rt_sleep(nano2count(500000)); // Wait 0.5ms
  int i;
  for (i = 0; i < bit_message_length; i++)
  {
    // Check if the current bit (starting from the MSB) is 1
    if (((message >> (63 - i)) & 0x01) == 1)
    {
      outb(0x11, LPT1);      // Set output high
      rt_sleep(bit_1_count); // Delay for 58µs
      outb(0x00, LPT1);      // Set output low
      rt_sleep(bit_1_count); // Delay for 58µs
    }
    else // 0-bit
    {
      outb(0x11, LPT1);      // Set output high
      rt_sleep(bit_0_count); // Delay for 100µs
      outb(0x00, LPT1);      // Set output low
      rt_sleep(bit_0_count); // Delay for 100µs
    }
  }
  outb(0x11, LPT1); // Final pulse
  rt_sem_signal(&bit_sem);
}

void send_magnetic_msg_task(void) // TODO: if not working then add (long arg)
{
  rt_printk("Starting magnetic msg task");
  while (1)
  {
    if (magnetic_msg_count > 0)
    {
      rt_sem_wait(&mag_sem[0]);
      unsigned long long telegram = buildMagneticTelegram(magnetic_msg_queue[0]);
      rt_sem_signal(&mag_sem[0]);
      send_bit_task(telegram, BIT_MESSAGE_LENGTH);

      // Shift messages up in the queue
      int i;
      MagneticDataConverter converter = {.unsigned_short = 0};
      for (i = 1; i < magnetic_msg_count; i++)
      {
        magnetic_msg_queue[i - 1] = magnetic_msg_queue[i];
        magnetic_msg_queue[i] = converter.magnetic_data;
      }
      magnetic_msg_count--;
    }
    rt_task_wait_period();
  }
}

void send_loco_msg_task(long i)
{
  rt_printk("Starting locomotive msg task %d", i);
  while (1)
  {
    if (i >= 0 && i < LOC_MSQ_SIZE)
    {
      rt_sem_wait(&loc_sem[i]);
      unsigned long long telegram = buildLocomotiveTelegram(locomotive_msg_queue[i]);
      rt_sem_signal(&loc_sem[i]);
      send_bit_task(telegram, BIT_MESSAGE_LENGTH);
    }
    rt_task_wait_period();
  }
}

void send_init_dcc(void) // TODO: if not working then add (long arg)
{
  rt_printk("Start initialization of dcc system...\n");
  unsigned long long reset_msg = buildResetAllTelegram();
  unsigned long long idle_msg = buildIdleTelegram();
  int i;
  for (i = 0; i < 20; i++)
  {
    send_bit_task(reset_msg, BIT_MESSAGE_LENGTH);
  }
  for (i = 0; i < 10; i++)
  {
    send_bit_task(idle_msg, BIT_MESSAGE_LENGTH);
  }
  rt_printk("Initialization of dcc system finished\n");
}

// EXPORT_SYMBOL(send_magnetic_msg_task);
// EXPORT_SYMBOL(send_loco_msg_task);
// EXPORT_SYMBOL(send_init_dcc);
