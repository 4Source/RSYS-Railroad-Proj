#ifndef RTAI_LINUX_COMMUNICATION_H
#define RTAI_LINUX_COMMUNICATION_H

#include "communication/railroad_communication.h"

#define FIFO_CMD 3
#define FIFO_ACK 4

int fifo_handler(unsigned int fifo);
void send_ack(uint16_t raw)


#endif