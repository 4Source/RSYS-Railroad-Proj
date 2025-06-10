#include "communication/railroad_communication.h"


static void send_bit_task(uint64_t message, int length)
{
  outb(0x00, LPT1); // set start voltlevel
  rt_sleep(nano2count(500000)); // wait 0.5ms
  int i;
  rt_mutex_lock(&bit_mutex);
  for (i = 0; i < length; i++)
  {
    if (((message >> (63 - i)) & 0x01) == 1) // 1-Bit
    {
      outb(0x11, LPT1);
      rt_sleep(nano2count(BIT_1_TIME));
      outb(0x00, LPT1);
      rt_sleep(nano2count(BIT_1_TIME));
    }
    else  // 0-Bit
    {
      outb(0x11, LPT1);
      rt_sleep(nano2count(BIT_0_TIME));
      outb(0x00, LPT1);
      rt_sleep(nano2count(BIT_0_TIME));
    }
  }
  rt_mutex_unlock(&bit_mutex);
  outb(0x11, LPT1);
}

static void send_magnetic_msg_task(long arg) {

if (magnetic_msg_count > 0) {
        rt_mutex_lock(&mag_sem[0]);
        MagneticTelegram telegram = buildMagneticTelegram(magnetic_msg_queue[0]);
        rt_mutex_unlock(&mag_sem[0]);
        send_bit_task((uint64_t)telegram, length);

        // Nachrücken
        int i;
        for (i = 1; i < magnetic_msg_count; i++) {
            magnetic_msg_queue[i - 1] = magnetic_msg_queue[i];
        }
        magnetic_msg_count--;
    }
    if (rt_task_delete(&magnetic_task) == 0) { //One-Shot Task beenden
      magnetic_task* = NULL;
    }
}


static void send_loco_msg_task(long i)
{
  while (1)
  {
    outb(0x00, LPT1);

    if (i >= 0 && i < locomotive_count)
    {
      rt_mutex_lock(&loc_sem[i]);
      LocomotiveTelegram telegram = buildLocomotiveTelegram(locomotive_msg_queue[i]);
      rt_mutex_unlock(&loc_sem[i]);
      send_bit_task((uint64_t)telegram, length);
    }

    outb(0x11, LPT1);
    rt_task_wait_period();
  }
}


