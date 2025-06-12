#ifndef RAILROAD_COMMUNICATION_H
#define RAILROAD_COMMUNICATION_H

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>

#include "telegram/idle.h"
#include "telegram/locomotive.h"
#include "telegram/magnetic.h"
#include "telegram/reset.h"

#define BIT_1_TIME 58000  /* 58 microseconds */
#define BIT_0_TIME 100000 /* 100 microseconds */
#define LPT1 0x378        /* Parallel port address */
#define BIT_MESSAGE_LENGTH 42

#define LOC_MSQ_SIZE 2
LocomotiveData locomotive_msg_queue[LOC_MSQ_SIZE] = {
    {.address = 1, .light = 0, .direction = 0, .speed = 0},
    // {.address = 2, .light = 0, .direction = 0, .speed = 0},
    {.address = 3, .light = 1, .direction = 1, .speed = 0},
};

#define MAG_MSQ_SIZE 4
int magnetic_msg_count = 0;
MagneticData magnetic_msg_queue[MAG_MSQ_SIZE] = {
    {.ack = 0, .address = 0, .control = 0, .device = 0, .enable = 0, .type = 0},
    {.ack = 0, .address = 0, .control = 0, .device = 0, .enable = 0, .type = 0},
    {.ack = 0, .address = 0, .control = 0, .device = 0, .enable = 0, .type = 0},
    {.ack = 0, .address = 0, .control = 0, .device = 0, .enable = 0, .type = 0},
};

static SEM bit_sem;
static SEM loc_sem[LOC_MSQ_SIZE];
static SEM mag_sem[MAG_MSQ_SIZE];

RT_TASK loco_tasks[LOC_MSQ_SIZE];
RT_TASK magnetic_task;
RT_TASK init_task;

/**
 * @brief Sends a bit-encoded message via a parallel port.
 *
 * This function transmits a message bit-by-bit using specific timing for logical '1' and '0'.
 * It uses a semaphore (bit_sem) to ensure exclusive access to the communication channel (LPT1).
 *
 * @param message:           The message to send, where each bit is checked starting from the most significant bit.
 * @param bit_message_length: The number of bits from the message to transmit.
 */
void send_bit_task(unsigned long long message, int bit_message_length);

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
void send_magnetic_msg_task(void);

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
 *           within the range [0, LOC_MSQ_SIZE) to ensure valid access.
 */
void send_loco_msg_task(long i);

/**
 * @brief Initialize the DCC system by transmitting reset and idle telegrams.
 *
 * This function performs the following steps:
 *  1. Sends the reset telegram repeatedly (20 times) to ensure that all devices
 *    in the system are reset.
 *  2. Sends the idle telegram repeatedly (10 times) to place the system in a stable, idle state.
 */
void send_init_dcc(void);

#endif