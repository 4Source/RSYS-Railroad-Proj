#ifndef RTAI_LINUX_COMMUNICATION_H
#define RTAI_LINUX_COMMUNICATION_H

#include "railroad_communication.h"

#define FIFO_CMD
#define FIFO_ACK 
#define ACK_BYTE 0xA5

int fifo_handler(unsigned int fifo);
void send_ack(uint16_t raw)


#endif