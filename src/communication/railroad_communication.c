#include "communication/railroad_communication.h"

void send_bit_task(unsigned long long message, int length)
{
  outb(0x00, LPT1);             // set start voltlevel
  rt_sleep(nano2count(500000)); // wait 0.5ms
  int i;
  rt_sem_wait(&bit_sem);
  for (i = 0; i < length; i++)
  {
    if (((message >> (63 - i)) & 0x01) == 1) // 1-Bit
    {
      outb(0x11, LPT1);
      rt_sleep(nano2count(BIT_1_TIME));
      outb(0x00, LPT1);
      rt_sleep(nano2count(BIT_1_TIME));
    }
    else // 0-Bit
    {
      outb(0x11, LPT1);
      rt_sleep(nano2count(BIT_0_TIME));
      outb(0x00, LPT1);
      rt_sleep(nano2count(BIT_0_TIME));
    }
  }
  rt_sem_signal(&bit_sem);
  outb(0x11, LPT1);
}

void send_magnetic_msg_task(long arg)
{

  if (magnetic_msg_count > 0)
  {
    rt_sem_wait(&mag_sem[0]);
    unsigned long long telegram = buildMagneticTelegram(magnetic_msg_queue[0]);
    rt_sem_signal(&mag_sem[0]);
    send_bit_task(telegram, length);

    // Nachrücken
    int i;
    for (i = 1; i < magnetic_msg_count; i++)
    {
      magnetic_msg_queue[i - 1] = magnetic_msg_queue[i];
    }
    magnetic_msg_count--;
  }
}

void send_loco_msg_task(long i)
{
  while (1)
  {
    outb(0x00, LPT1);

    if (i >= 0 && i < locomotive_count)
    {
      rt_sem_wait(&loc_sem[i]);
      unsigned long long telegram = buildLocomotiveTelegram(locomotive_msg_queue[i]);
      rt_sem_signal(&loc_sem[i]);
      send_bit_task(telegram, length);
    }

    outb(0x11, LPT1);
    rt_task_wait_period();
  }
}

EXPORT_SYMBOL(send_magnetic_msg_task);
EXPORT_SYMBOL(send_loco_msg_task);
