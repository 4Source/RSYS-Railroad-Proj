#ifndef RESET_H
#define RESET_H

#include <stdint.h>

/**
 * @struct ResetAllTelegram
 * @brief Represents the structure of a Digital Decoder Reset Packet.
 *
 * This structure defines the bit fields for a reset telegram, which is used to reset
 * all digital decoders. When a decoder receives this packet, it erases all volatile memory
 * (including speed and direction data) and returns to its normal power-up state. If the
 * decoder is operating a locomotive at a non-zero speed, it will bring the locomotive to
 * an immediate stop.
 *
 * bit Field Descriptions:
 * - reserved (22 bits): Reserved for future use. [bits 0 - 21]
 * - stop_bit (1 bit): Stop bit of the telegram. Fixed value: 1. [bit 22]
 * - checksum (8 bits): An 8-bit field for the checksum value. Checksum is the XOR of the address and command bytes [bits 23 - 30]
 * - start_bit_cs (1 bit): Start bit for the checksum field. Fixed value: 0. [bit 31]
 * - command (8 bits): Command field. Fixed value: 0 (Reset command). [bits 32 - 39]
 * - start_bit_cmd (1 bit): Start bit for the command field. Fixed value: 0. [bit 40]
 * - address (8 bits): Address field. Fixed value: 0 (broadcast address). [bits 41 - 48]
 * - start_bit_address (1 bit): Start bit for the address field. Fixed value: 0. [bit 49]
 * - preamble (14 bits): Preamble for synchronization. Fixed value: 0b11111111111111 (0x3FFF). [bits 50 - 63]
 */
typedef struct
{
    uint64_t reserved : 22;         // [bit 0 - 21]     Reserved for future use.
    uint64_t stop_bit : 1;          // [bit 22]         Stop bit of the telegram. Fixed value: 1.
    uint64_t checksum : 8;          // [bit 23 - 30]    An 8-bit field for the checksum value.
    uint64_t start_bit_cs : 1;      // [bit 31]         Start bit for the checksum field. Fixed value: 0.
    uint64_t command : 8;           // [bit 32 - 39]    Command field. Fixed value: 0 (Reset command).
    uint64_t start_bit_cmd : 1;     // [bit 40]         Start bit for the command field. Fixed value: 0.
    uint64_t address : 8;           // [bit 41 - 48]    Address field. Fixed value: 0 (broadcast address).
    uint64_t start_bit_address : 1; // [bit 49]         Start bit for the address field. Fixed value: 0.
    uint64_t preamble : 14;         // [bit 50 - 63]    Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
} ResetAllTelegram;

/**
 * @function buildResetAllTelegram
 * @brief Constructs and initializes a ResetAllTelegram structure.
 *
 * This function creates and initializes a ResetAllTelegram structure with the fixed
 * values defined for a Digital Decoder Reset Packet. The structure adheres to the
 * format specified in the documentation and ensures proper synchronization and
 * error detection.
 *
 * @return ResetAllTelegram A fully initialized reset telegram structure.
 */
ResetAllTelegram buildResetAllTelegram();

#endif