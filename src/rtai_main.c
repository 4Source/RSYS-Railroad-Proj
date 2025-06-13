#include <linux/module.h>
#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>
#include <rtai_fifos.h>

#include "telegram/idle.h"
#include "telegram/locomotive.h"
#include "telegram/magnetic.h"
#include "telegram/reset.h"
#include "config.h"

#define FIFO_CMD 3
#define FIFO_ACK 4
#define STACK_SIZE 4096
#define FIFO_SIZE 1024
#define PERIOD_TIMER 1000000
#define PERIOD_MAG_TASK 50000000
#define PERIOD_LOC_TASK 5000000

#define BIT_1_TIME 58000  /* 58 microseconds */
#define BIT_0_TIME 100000 /* 100 microseconds */
#define LPT1 0x378        /* Parallel port address */
#define BIT_MESSAGE_LENGTH 42

LocomotiveData locomotive_msg_queue[LOC_SIZE];
memcpy(locomotive_msg_queue, &locomotives_user.data, LOC_SIZE * sizeof(locomotive_msg_queue));

int magnetic_msg_count = 0;
MagneticData magnetic_msg_queue[MAG_SIZE];
memcpy(magnetic_msg_queue, &magnetic_user.data, MAG_SIZE * sizeof(magnetic_msg_queue));

static SEM bit_sem;
static SEM loc_sem[LOC_SIZE];
static SEM mag_sem[MAG_SIZE];

RT_TASK loco_tasks[LOC_SIZE];
RT_TASK magnetic_task;
RT_TASK init_task;

/**
 * send_ack - Send an acknowledgement with the ACK bit set.
 *
 * This function modifies the provided raw data by setting its 15th bit,
 * marking it as an acknowledgement. It then sends the modified data to the FIFO_ACK channel.
 *
 * @param raw: An unsigned short value representing the data before setting
 *             the ACK flag. Upon modification, the 15th bit is set.
 */
void send_ack(unsigned short raw)
{
  // Set bit 15 as the ACK flag.
  raw |= (1 << 15);

  int result = rtf_put(FIFO_ACK, &raw, sizeof(raw));

  if (result < 0)
  {
    rt_printk("Error sending ACK: rtf_put returned %d.\n", result);
  }
  else if (result != sizeof(raw))
  {
    rt_printk("ACK could not be sent completely. Expected: %lu, received: %d.\n", sizeof(raw), result);
  }
}

/**
 * Finds the index of a locomotive in the locomotive message queue by its address.
 *
 * This function iterates through the globally defined locomotive message queue
 * and compares each entry's address with the address provided in the given LocomotiveData structure.
 * If a matching address is found, the index of that entry in the queue is returned.
 * If no match is found, the function returns -1.
 *
 * @param loc A LocomotiveData structure containing the address to locate within the message queue.
 * @return The index of the matching locomotive in the message queue if found; otherwise, -1.
 */
int findIndexOfLocAddress(LocomotiveData loc)
{
  int i;
  for (i = 0; i < LOC_SIZE; i++)
  {
    if (locomotive_msg_queue[i].address == loc.address)
    {
      return i;
    }
  }
  return -1;
}

/**
 * @brief Finds the index of a magnetic message in the magnetic message queue.
 *
 * This function iterates over the global magnetic_msg_queue array to locate the entry
 * that matches both the address and device values of the provided MagneticData structure.
 *
 * @param mag A MagneticData structure that contains the address and device used for the lookup.
 *
 * @return The index of the matching magnetic message if found; otherwise, -1.
 */
int findIndexOfMagAddress(MagneticData mag)
{
  int i;
  for (i = 0; i < MAG_SIZE; i++)
  {
    if (magnetic_msg_queue[i].address == mag.address && magnetic_msg_queue[i].device == mag.device)
    {
      return i;
    }
  }
  return -1;
}

/**
 * Process a FIFO command based on its type.
 *
 * This function reads a command from a FIFO using rtf_get and interprets the first two
 * bytes as an unsigned short. It extracts bits 13-14 to determine the command type and
 * processes the command accordingly.
 *
 * @param fifo - An unsigned int representing the FIFO identifier (used for handling the FIFO), though not explicitly processed in this function.
 *
 * @return 0 on completion.
 */
int fifo_handler(unsigned int fifo)
{
  char command[FIFO_SIZE];
  int r;
  unsigned short raw;

  // Get data from FIFO_CMD channel.
  r = rtf_get(FIFO_CMD, command, sizeof(command) - 1);
  rt_printk("Fifo handler received %d bytes\n", r);

  if (r >= sizeof(unsigned short))
  {
    // Extract the 2-byte raw command
    memcpy(&raw, command, sizeof(unsigned short));
    rt_printk("Fifo handler received command 0x%04X\n", raw);

    // Extract command type from bits 13-14. (Shift and mask with 0x3)
    unsigned short type = (raw >> 13) & 0x3;

    if (type == 0x1) // Locomotive command
    {
      rt_printk("Command is of type: locomotive");
      // Use union conversion to obtain message structure from raw
      LocomotiveDataConverter converter = {.unsigned_short = raw};

      // Find correct index in locomotive_msg_queue
      int index = findIndexOfLocAddress(converter.locomotive_data);
      rt_printk("Index of address %d is %d", converter.locomotive_data.address, index);
      if (index >= 0)
      {
        // Protect access to message queue using semaphore
        rt_sem_wait(&loc_sem[index]);
        locomotive_msg_queue[index] = converter.locomotive_data;
        rt_sem_signal(&loc_sem[index]);
        rt_printk("Locomotive Addr %d: Speed=%d Dir=%d Light=%d\n", converter.locomotive_data.address, converter.locomotive_data.speed, converter.locomotive_data.direction, converter.locomotive_data.light);
        send_ack(raw);
      }
      else
      {
        rt_printk("Invalid locomotive address: %d\n", converter.locomotive_data.address);
      }
    }
    else if (type == 0x2) // Magnetic command
    {
      rt_printk("Command is of type: magnetic");
      MagneticDataConverter converter = {.unsigned_short = raw};

      // Find matching message in magnetic queue
      int index = findIndexOfMagAddress(converter.magnetic_data);
      rt_printk("Index of address %d device %d is %d", converter.magnetic_data.address, converter.magnetic_data.device, index);
      if (index >= 0)
      {
        rt_sem_wait(&mag_sem[index]);
        magnetic_msg_queue[index] = converter.magnetic_data;
        rt_sem_signal(&mag_sem[index]);
        rt_printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n", converter.magnetic_data.address, converter.magnetic_data.device, converter.magnetic_data.enable, converter.magnetic_data.control);
        send_ack(raw);
      }
      else if (magnetic_msg_count < MAG_SIZE)
      {
        // If not found, use next free slot
        rt_sem_wait(&mag_sem[magnetic_msg_count]);
        magnetic_msg_queue[magnetic_msg_count] = converter.magnetic_data;
        rt_sem_signal(&mag_sem[magnetic_msg_count]);
        magnetic_msg_count++;
        rt_printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n", converter.magnetic_data.address, converter.magnetic_data.device, converter.magnetic_data.enable, converter.magnetic_data.control);
        send_ack(raw);
      }
      else
      {
        rt_printk("Magnetic queue full!\n");
      }
    }
    else
    {
      rt_printk("Unknown message type: %d\n", type);
    }
  }
  else
  {
    rt_printk("Invalid FIFO data (only %d bytes)\n", r);
  }

  return 0;
}

/**
 * @brief Sends a bit-encoded message via a parallel port.
 *
 * This function transmits a message bit-by-bit using specific timing for logical '1' and '0'.
 * It uses a semaphore (bit_sem) to ensure exclusive access to the communication channel (LPT1).
 *
 * @param message:           The message to send, where each bit is checked starting from the most significant bit.
 * @param bit_message_length: The number of bits from the message to transmit.
 */
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
  MagneticConverter converter = {.magnetic_telegram = telegram};

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
  LocomotiveConverter converter = {.locomotive_telegram = telegram};

  return converter.unsigned_long_long;
}

/**
 * @brief Constructs and initializes an IdleTelegram structure.
 *
 * This function creates and initializes an IdleTelegram structure with the fixed
 * values defined for a Digital Decoder Idle Packet. The structure adheres to the
 * format specified in the documentation and ensures proper synchronization and
 * error detection.
 *
 * @return IdleTelegram A fully initialized idle telegram structure.
 */
unsigned long long buildIdleTelegram(void)
{
  IdleTelegram telegram = {
      .preamble = 0b11111111111111, // Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
      .start_bit_address = 0,       // Start bit for the address field. Fixed value: 0.
      .address = 0xFF,              // Address field. Fixed value: 0b11111111 (0xFF) (idle packet address).
      .start_bit_cmd = 0,           // Start bit for the command field. Fixed value: 0.
      .command = 0,                 // Command field. Fixed value: 0 (Idle command).
      .start_bit_cs = 0,            // Start bit for the checksum field. Fixed value: 0.
      .checksum = 0xFF,             // Checksum value.
      .stop_bit = 1,                // Stop bit of the telegram. Fixed value: 1.
      .reserved = 0                 // Reserved field, set to 0.
  };

  IdleConverter converter = {.idle_telegram = telegram};

  return converter.unsigned_long_long;
}

/**
 * @brief Constructs and initializes a ResetAllTelegram structure.
 *
 * This function creates and initializes a ResetAllTelegram structure with the fixed
 * values defined for a Digital Decoder Reset Packet. The structure adheres to the
 * format specified in the documentation and ensures proper synchronization and
 * error detection.
 *
 * @return ResetAllTelegram A fully initialized reset telegram structure.
 */
unsigned long long buildResetAllTelegram(void)
{
  ResetAllTelegram telegram = {
      .preamble = 0b11111111111111, // Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
      .start_bit_address = 0,       // Start bit for the address field. Fixed value: 0.
      .address = 0,                 // Address field. Fixed value: 0 (broadcast address).
      .start_bit_cmd = 0,           // Start bit for the command field. Fixed value: 0.
      .command = 0,                 // Command field. Fixed value: 0 (Reset command).
      .start_bit_cs = 0,            // Start bit for the checksum field. Fixed value: 0.
      .checksum = 0,                // Checksum value.
      .stop_bit = 1,                // Stop bit of the telegram. Fixed value: 1.
      .reserved = 0                 // Reserved field, set to 0
  };

  ResetConverter converter = {.reset_telegram = telegram};

  return converter.unsigned_long_long;
}

/**
 * @brief Task responsible for continuously sending magnetic messages.
 *
 * This function runs an infinite loop that monitors the magnetic message queue.
 * When there is at least one message available, it:
 *   - Waits for semaphore synchronization to safely access the magnetic message queue.
 *   - Builds a telegram from the current magnetic message.
 *   - Releases the semaphore.
 *   - Sends the built telegram using the send_bit_task function.
 *   - Shifts all remaining messages in the queue one position forward to remove
 *     the sent message, and decrements the magnetic message count.
 * Finally, the task waits until the next cycle using rt_task_wait_period().
 */
void send_magnetic_msg_task(long arg)
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

/**
 * @brief Task responsible for continuously sending locomotive messages.
 *
 * This function runs an infinite loop that sends one of the locomotive messages
 * for which locomotive is defined by the parameter i that is provided to the function:
 *   - It waits on the corresponding semaphore (loc_sem[i]) to safely access the locomotive message data.
 *   - It builds a telegram using the message from the locomotive_msg_queue at index i.
 *   - It signals the semaphore to release it after reading the message.
 *   - It sends the built telegram using send_bit_task, with a fixed message length defined by BIT_MESSAGE_LENGTH.
 *
 * @param i: An integer representing the index into loc_sem and locomotive_msg_queue arrays. Must be
 *           within the range [0, LOC_SIZE) to ensure valid access.
 */
void send_loco_msg_task(long i)
{
  rt_printk("Starting locomotive msg task %d", i);
  while (1)
  {
    if (i >= 0 && i < LOC_SIZE)
    {
      rt_sem_wait(&loc_sem[i]);
      unsigned long long telegram = buildLocomotiveTelegram(locomotive_msg_queue[i]);
      rt_sem_signal(&loc_sem[i]);
      send_bit_task(telegram, BIT_MESSAGE_LENGTH);
    }
    rt_task_wait_period();
  }
}

/**
 * @brief Initialize the DCC system by transmitting reset and idle telegrams.
 *
 * This function performs the following steps:
 *  1. Sends the reset telegram repeatedly (20 times) to ensure that all devices
 *    in the system are reset.
 *  2. Sends the idle telegram repeatedly (10 times) to place the system in a stable, idle state.
 */
void send_init_dcc(long arg)
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
  for (i = 0; i < LOC_SIZE; i++)
  {
    rt_sem_init(&loc_sem[i], 1);
  }
  for (i = 0; i < MAG_SIZE; i++)
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
  for (i = 0; i < LOC_SIZE; i++)
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
  for (i = 0; i < LOC_SIZE; i++)
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
  for (i = 0; i < LOC_SIZE; i++)
  {
    rt_task_delete(&loco_tasks[i]);
  }

  rtf_destroy(FIFO_ACK);
  rtf_destroy(FIFO_CMD);

  rt_sem_delete(&bit_sem);
  for (i = 0; i < LOC_SIZE; i++)
  {
    rt_sem_delete(&loc_sem[i]);
  }
  for (i = 0; i < MAG_SIZE; i++)
  {
    rt_sem_delete(&mag_sem[i]);
  }

  rt_umount();
  rt_printk("Unloading module\n");
}

module_init(send_init);
module_exit(send_exit);

MODULE_LICENSE("GPL");