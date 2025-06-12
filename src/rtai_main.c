#include <linux/kernel.h>
#include <linux/module.h>
#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>
#include <rtai_fifos.h>

#include "telegram/locomotive.h"
#include "telegram/magnetic.h"

#define FIFO_CMD 3
#define FIFO_ACK 4
#define STACK_SIZE 4096
#define FIFO_SIZE 1024
#define PERIOD_TIMER 20000000
#define PERIOD_MAG_TASK 70000000
#define PERIOD_LOC_TASK 60000000

#define BIT_1_TIME 58000  /* 58 microseconds */
#define BIT_0_TIME 100000 /* 100 microseconds */
#define LPT1 0x378        /* Parallel port address */
#define LOC_MSQ_SIZE 3
#define MAG_MSQ_SIZE 4
#define BIT_MESSAGE_LENGTH 42

static SEM bit_sem;
static SEM loc_sem[LOC_MSQ_SIZE];
static SEM mag_sem[MAG_MSQ_SIZE];

RT_TASK loco_tasks[LOC_MSQ_SIZE];
RT_TASK *magnetic_task = NULL;

const int locomotive_count = 3;
int magnetic_msg_count = 0;
LocomotiveData locomotive_msg_queue[LOC_MSQ_SIZE] = {
    {.address = 1, .light = 0, .direction = 0, .speed = 0},
    {.address = 2, .light = 0, .direction = 0, .speed = 0},
    {.address = 3, .light = 1, .direction = 1, .speed = 15},
};
MagneticData magnetic_msg_queue[MAG_MSQ_SIZE] = {
    {.address = 0, .device = 1, .enable = 0, .control = 0},
    {.address = 0, .device = 2, .enable = 0, .control = 0},
    {.address = 0, .device = 3, .enable = 0, .control = 0},
    {.address = 0, .device = 4, .enable = 0, .control = 0},
};

void send_ack(unsigned short raw)
{
  // Set bit 15 (ACK bit)
  raw |= (1 << 15);

  int result = rtf_put(FIFO_ACK, &raw, sizeof(raw));

  if (result < 0)
  {
    printk("Error sending ACK: rtf_put returned %d.\n", result);
  }
  else if (result != sizeof(raw))
  {
    printk("ACK could not be sent completely. Expected: %lu, received: %d.\n", sizeof(raw), result);
  }
}

int fifo_handler(unsigned int fifo)
{
  char command[1024];
  int r;
  unsigned short raw;

  r = rtf_get(FIFO_CMD, command, sizeof(command) - 1);
  if (r >= sizeof(unsigned short))
  {
    memcpy(&raw, command, sizeof(unsigned short));

    // Check type (bits 13-14)
    unsigned short type = (raw >> 13) & 0x3;

    if (type == 0x1)
    {
      LocomotiveDataConverter converter;
      converter.unsigned_short = raw;
      // Locomotive
      LocomotiveData loco = converter.locomotive_data;

      if (loco.address <= LOC_MSQ_SIZE && loco.address > 0)
      {
        rt_sem_wait(&loc_sem[loco.address - 1]);
        locomotive_msg_queue[loco.address - 1] = loco;
        rt_sem_signal(&loc_sem[loco.address - 1]);
        printk("Locomotive Addr %d: Speed=%d Dir=%d Light=%d\n", loco.address, loco.speed, loco.direction, loco.light);
        send_ack(raw);
      }
      else
      {
        printk("Invalid locomotive address: %d\n", loco.address);
      }
    }
    else if (type == 0x2)
    {
      MagneticDataConverter converter;
      converter.unsigned_short = raw;
      // Magnetic
      MagneticData mag = converter.magnetic_data;

      if (magnetic_msg_count < MAG_MSQ_SIZE)
      // TODO: override existing
      {
        rt_sem_wait(&mag_sem[magnetic_msg_count]);
        magnetic_msg_queue[magnetic_msg_count] = mag;
        rt_sem_signal(&mag_sem[magnetic_msg_count]);
        magnetic_msg_count++;
        printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n", mag.address, mag.device, mag.enable, mag.control);
        send_ack(raw);
      }
      else
      {
        printk("Magnetic queue full!\n");
      }
    }
    else
    {
      printk("Unknown message type: %d\n", type);
    }
  }
  else
  {
    printk("Invalid FIFO data (only %d bytes)\n", r);
  }

  return 0;
}

void send_bit_task(unsigned long long message, int bit_message_length)
{
  RTIME bit_1_count = nano2count(BIT_1_TIME);
  RTIME bit_0_count = nano2count(BIT_0_TIME);

  outb(0x00, LPT1);             // Set initial voltage level
  rt_sleep(nano2count(500000)); // Wait 0.5ms
  int i;
  rt_sem_wait(&bit_sem);
  for (i = 0; i < bit_message_length; i++)
  {
    if (((message >> (63 - i)) & 0x01) == 1) // 1-bit
    {
      outb(0x11, LPT1);
      rt_sleep(bit_1_count);
      outb(0x00, LPT1);
      rt_sleep(bit_1_count);
    }
    else // 0-bit
    {
      outb(0x11, LPT1);
      rt_sleep(bit_0_count);
      outb(0x00, LPT1);
      rt_sleep(bit_0_count);
    }
  }
  rt_sem_signal(&bit_sem);
  outb(0x11, LPT1);
}

/**
 * @brief Constructs a MagneticTelegram from the given MagneticData.
 *
 * This function takes a MagneticData structure as input and constructs
 * a corresponding MagneticTelegram structure for communication.
 *
 * @param data The MagneticData structure containing the magnetic data.
 * @return A MagneticTelegram structure constructed from the input data.
 */
unsigned long long buildMagneticTelegram(MagneticData data)
{
  // Extract the high bits of the address and invert them (bits 8 - 6 of the address)
  unsigned short address_high = ~((data.address >> 6) & 0b111);

  // Extract the low bits of the address (bits 5 - 0 of the address)
  char address_low = (data.address & 0b111111);

  // Construct the address byte:
  // - The MSB is set to 0b10
  // - The lower 6 bits are taken from the low bits of the address
  char address = 0b10000000 | address_low;

  // Construct the command byte:
  // - The MSB is set to 0b1
  // - The high bits of the address (inverted) are shifted to bits 4 - 6
  // - The control bit is shifted to bit 3
  // - The device type occupies bits 2 - 1
  // - The enable bit occupies bit 0
  char command = 0b10000000 | (address_high << 4) | (data.control << 3) | (data.device << 1) | data.enable;

  // Calculate the checksum as the XOR of the address and command bytes
  char checksum = address ^ command;

  // Build the MagneticTelegram structure with the calculated and fixed values
  MagneticTelegram telegram = {
      .preamble = 0b11111111111111, // Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
      .start_bit_address = 0,       // Start bit for the address field
      .address_bits_7_6 = 0b10,     // Fixed high bits (7 - 6) of the address
      .address_low = address_low,   // Low bits (5 - 0) of the address
      .start_bit_cmd = 0,           // Start bit for the command field
      .cmd_bit_7 = 1,               // Fixed high bit (7) of the command
      .address_high = address_high, // High bits (8 - 6) of the address
      .control = data.control,      // Control field from the input data
      .device = data.device,        // Device field from the input data
      .enable = data.enable,        // Enable field from the input data
      .start_bit_cs = 0,            // Start bit for the checksum field
      .checksum = checksum,         // Calculated checksum value
      .stop_bit = 1,                // Fixed stop bit
      .reserved = 0                 // Reserved field, set to 0
  };
  MagneticConverter converter;
  converter.magnetic_telegram = telegram;

  return converter.unsigned_long_long;
}

/**
 * @brief Constructs a LocomotiveTelegram from the given LocomotiveData.
 *
 * This function takes a LocomotiveData structure as input and constructs
 * a corresponding LocomotiveTelegram structure for communication.
 *
 * @param data The LocomotiveData structure containing the locomotive data.
 * @return A LocomotiveTelegram structure constructed from the input data.
 */
unsigned long long buildLocomotiveTelegram(LocomotiveData data)
{
  // Set the MSB to (0b0)
  char address = 0b00000000 | data.address;

  // Construct the command byte:
  // - The first two bits are set to 0b01
  // - The direction bit is shifted to bit 5
  // - The light bit is shifted to bit 4
  // - The speed value occupies bits 0 - 3
  char command = 0b01000000 | (data.direction << 5) | (data.light << 4) | data.speed;

  // Calculate the checksum as the XOR of the address and command bytes
  char checksum = address ^ command;

  // Build the LocomotiveTelegram structure with the calculated values
  LocomotiveTelegram telegram = {
      .preamble = 0b11111111111111, // Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
      .start_bit_address = 0,       // Start bit for the address section
      .address_bit_7 = 0,           // Reserved bit in the address section
      .address = data.address,      // Address of the locomotive
      .start_bit_cmd = 0,           // Start bit for the command section
      .cmd_bits_7_6 = 0b01,         // Fixed command bits
      .direction = data.direction,  // Direction of the locomotive
      .light = data.light,          // Light status (on/off)
      .speed = data.speed,          // Speed of the locomotive
      .start_bit_cs = 0,            // Start bit for the checksum section
      .checksum = checksum,         // Calculated checksum value.
      .stop_bit = 1,                // Stop bit
      .reserved = 0,                // Reserved field
  };
  LocomotiveConverter converter;
  converter.locomotive_telegram = telegram;

  return converter.unsigned_long_long;
}

void send_magnetic_msg_task(long arg)
{
  if (magnetic_msg_count > 0)
  {
    rt_sem_wait(&mag_sem[0]);
    unsigned long long telegram = buildMagneticTelegram(magnetic_msg_queue[0]);
    rt_sem_signal(&mag_sem[0]);
    send_bit_task(telegram, BIT_MESSAGE_LENGTH);

    // Shift queue elements
    int i;
    for (i = 1; i < magnetic_msg_count; i++)
    {
      magnetic_msg_queue[i - 1] = magnetic_msg_queue[i];
    }
    magnetic_msg_count--;
    if (magnetic_msg_count >= 0)
    {
      MagneticDataConverter converter;
      converter.unsigned_short = 0;
      magnetic_msg_queue[magnetic_msg_count] = converter.magnetic_data;
    }
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
      send_bit_task(telegram, BIT_MESSAGE_LENGTH);
    }

    outb(0x11, LPT1);
    rt_task_wait_period();
  }
}

static __init int send_init(void)
{
  rt_mount_rtai();

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

  if (rtf_create(FIFO_CMD, FIFO_SIZE) - FIFO_SIZE != 0)
  {
    rt_printk("Failed to create cmd fifo!\n");
    return -1;
  }
  int handler_res = rtf_create_handler(FIFO_CMD, &fifo_handler);
  if (handler_res != 0)
  {
    rt_printk("Failed to create cmd fifo handler!\n");
    return handler_res;
  }
  if (rtf_create(FIFO_ACK, FIFO_SIZE) - FIFO_SIZE != 0)
  {
    rt_printk("Failed to create ack fifo!\n");
    return -1;
  }

  int task_init_res = rt_task_init(magnetic_task, send_magnetic_msg_task, 0, STACK_SIZE, 2, 0, 0);
  if (task_init_res != 0)
  {
    rt_printk("Failed to init magnetic task!\n");
    return task_init_res;
  }
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    task_init_res = rt_task_init(&loco_tasks[i], send_loco_msg_task, i, STACK_SIZE, 1, 0, 0);
    if (task_init_res != 0)
    {
      rt_printk("Failed to init locomotive %d task!\n", i);
      return task_init_res;
    }
  }

  rt_set_periodic_mode();
  start_rt_timer(nano2count(PERIOD_TIMER));

  int task_make_res = rt_task_make_periodic(magnetic_task, rt_get_time() + nano2count(1000000000), nano2count(PERIOD_MAG_TASK));
  if (task_make_res != 0)
  {
    rt_printk("Failed to make periodic magnetic task!\n");
    return task_make_res;
  }
  for (i = 0; i < LOC_MSQ_SIZE; i++)
  {
    task_make_res = rt_task_make_periodic(&loco_tasks[i], rt_get_time() + nano2count(1000000000), nano2count(PERIOD_LOC_TASK + i));
    if (task_make_res != 0)
    {
      rt_printk("Failed to make periodic locomotive %d task!\n", i);
      return task_make_res;
    }
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

  rt_umount_rtai();
  rt_printk("Unloading module\n");
}

module_init(send_init);
module_exit(send_exit);

MODULE_LICENSE("GPL");