#ifndef LOCOMOTIVE_H
#define LOCOMOTIVE_H

#include <stdint.h>

/**
 * @struct LocomotiveData
 * @brief Represents the locomotive data with bit fields for various parameters.
 *
 * This structure contains the following fields:
 * - speed: A 4-bit field representing the speed of the locomotive (0 to 15).
 * - light: A 1-bit field indicating whether the locomotive's light is on (0 or 1).
 * - direction: A 1-bit field specifying the direction of the locomotive (0 or 1).
 * - address: A 7-bit field representing the address of the locomotive (0 to 127).
 * - reserved: A 3-bit reserved field for future use.
 */
typedef struct
{
    uint16_t speed : 4;     // [bit 0 - 3]    Speed of the locomotive. Values: 0 - 15 (0xF).
    uint16_t light : 1;     // [bit 4]        Indicates if the light is on. Values: 0, 1.
    uint16_t direction : 1; // [bit 5]        Direction of the locomotive. Values: 0, 1.
    uint16_t address : 7;   // [bit 6 - 12]   Address of the locomotive. Values: 0 - 127 (0x7F).
    uint16_t reserved : 3;  // [bit 13 - 15]  Reserved for future use.
} LocomotiveData;

/**
 * @struct Locomotive
 * @brief Combines locomotive data with an alias for identification.
 *
 * This structure contains:
 * - data: A LocomotiveData structure holding the locomotive data.
 * - alias: A constant character pointer for the alias name.
 */
typedef struct
{
    LocomotiveData data; // A LocomotiveData structure holding the locomotive data.
    const char *alias;   // A constant character pointer for the alias name.
} Locomotive;

/**
 * @struct LocomotiveTelegram
 * @brief Represents a locomotive telegram with detailed bit fields.
 *
 * This structure is used to construct a locomotive telegram for communication.
 * It contains the following fields:
 * - reserved: A 22-bit reserved field for future use.
 * - stop_bit: A 1-bit field indicating the stop_bit of the telegram.
 * - checksum: An 8-bit field for the checksum value.
 * - start_bit_cs: A 1-bit field for the start bit of the checksum.
 * - speed: A 4-bit field representing the speed of the locomotive.
 * - light: A 1-bit field indicating whether the locomotive's light is on.
 * - direction: A 1-bit field specifying the direction of the locomotive.
 * - cmd_bits_7_6: A 2-bit field for command bits 7 and 6.
 * - start_bit_cmd: A 1-bit field for the start bit of the command.
 * - address: A 7-bit field representing the address of the locomotive.
 * - address_bit_7: A 1-bit field for the 7th bit of the address.
 * - start_bit_address: A 1-bit field for the start bit of the address.
 * - preamble: A 14-bit field for the preamble.
 */
typedef struct
{
    uint64_t reserved : 22;         // [bit 0 - 21]    Reserved for future use.
    uint64_t stop_bit : 1;               // [bit 22]        Indicates the stop_bit of the telegram. Values: 0, 1.
    uint64_t checksum : 8;          // [bit 23 - 30]   Checksum value for error detection. Values: 0 - 255 (0xFF).
    uint64_t start_bit_cs : 1;      // [bit 31]        Start bit for checksum field. Values: 0, 1.
    uint64_t speed : 4;             // [bit 32 - 35]   Speed of the locomotive. Values: 0 - 15 (0xF).
    uint64_t light : 1;             // [bit 36]        Indicates if the light is on. Values: 0, 1.
    uint64_t direction : 1;         // [bit 37]        Direction of the locomotive. Values: 0, 1.
    uint64_t cmd_bits_7_6 : 2;      // [bit 38 - 39]   Command bits 7 and 6. Values: 0 - 3.
    uint64_t start_bit_cmd : 1;     // [bit 40]        Start bit for the command field. Values: 0, 1.
    uint64_t address : 7;           // [bit 41 - 47]   Address of the locomotive. Values: 0 - 127 (0x7F).
    uint64_t address_bit_7 : 1;     // [bit 48]        7th bit of the address. Values: 0, 1.
    uint64_t start_bit_address : 1; // [bit 49]        Start bit for the address field. Values: 0, 1.
    uint64_t preamble : 14;         // [bit 50 - 63]   Preamble for synchronization. Values: 0 - 16383 (0x3FFF).
} LocomotiveTelegram;

/**
 * @brief Constructs a LocomotiveTelegram from the given LocomotiveData.
 *
 * This function takes a LocomotiveData structure as input and constructs
 * a corresponding LocomotiveTelegram structure for communication.
 *
 * @param data The LocomotiveData structure containing the locomotive data.
 * @return A LocomotiveTelegram structure constructed from the input data.
 */
LocomotiveTelegram buildLocomotiveTelegram(LocomotiveData data);

#endif