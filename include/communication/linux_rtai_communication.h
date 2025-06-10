#ifndef LINUX_RTAI_COMMUNICATION_H
#define LINUX_RTAI_COMMUNICATION_H

#include <stdint.h>

/**
 * @brief Sends a 16-bit data value with acknowledgement.
 *
 * This function attempts to send the provided 16-bit data to a receiving process
 * over the RTAI communication framework and waits for an acknowledgement. In case
 * of failure, the transmission is retried up to the number of attempts specified.
 *
 * @param data The data value to be transmitted.
 * @param attempts The maximum number of transmission attempts (defaults to 3).
 * @return int Returns a status code indicating success or failure of the transmission.
 */
int send_with_ack(uint16_t data, int attempts);

#endif