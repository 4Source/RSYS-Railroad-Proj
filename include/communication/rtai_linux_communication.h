#ifndef RTAI_LINUX_COMMUNICATION_H
#define RTAI_LINUX_COMMUNICATION_H

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>
#include <rtai_fifos.h>

#include "telegram/locomotive.h"
#include "telegram/magnetic.h"

#define FIFO_CMD 3
#define FIFO_ACK 4
#define FIFO_SIZE 1024
#define STACK_SIZE 4096

/**
 * send_ack - Send an acknowledgement with the ACK bit set.
 *
 * This function modifies the provided raw data by setting its 15th bit,
 * marking it as an acknowledgement. It then sends the modified data to the FIFO_ACK channel.
 *
 * @param raw: An unsigned short value representing the data before setting
 *             the ACK flag. Upon modification, the 15th bit is set.
 */
void send_ack(unsigned short raw);

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
int findIndexOfLocAddress(LocomotiveData loc);

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
int findIndexOfMagAddress(MagneticData mag);

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
int fifo_handler(unsigned int fifo);

#endif