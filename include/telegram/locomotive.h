#ifndef LOCOMOTIVE_H
#define LOCOMOTIVE_H

/**
 * @struct LocomotiveData
 * @brief Represents the locomotive data with bit fields for various parameters.
 *
 * This structure contains the following fields:
 * - speed (4 bits): Representing the speed of the locomotive (0 to 15).
 * - light (1 bit): Indicating whether the locomotive's light is on (0 or 1).
 * - direction (1 bit): Specifying the direction of the locomotive (0 or 1).
 * - address (7 bits): Representing the address of the locomotive (0 to 127).
 * - type (2 bits): Type of data (01 = Locomotive, 10 = Magnetic).
 * - ack (1 bit): Acknowledge flag (0 = not acknowledged, 1 = acknowledged).
 */
typedef struct
{
    unsigned short speed : 4;     // [bit 0 - 3]    Speed of the locomotive. Values: 0 - 15 (0xF).
    unsigned short light : 1;     // [bit 4]        Indicates if the light is on. Values: 0 (off), 1 (on).
    unsigned short direction : 1; // [bit 5]        Direction of the locomotive. Values: 0 (backwards), 1 (forwards).
    unsigned short address : 7;   // [bit 6 - 12]   Address of the locomotive. Values: 0 - 127 (0x7F).
    unsigned short type : 2;      // [bit 13 - 14]  Type of data: 01 (Locomotive), 10 (Magnetic)
    unsigned short ack : 1;       // [bit 15]       Acknowledge: 0 (not), 1 (ack)
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
    char alias[20];      // A character array with a fixed length of 20 for the alias name.
} Locomotive;

/**
 * @struct LocomotiveTelegram
 * @brief Represents a locomotive telegram with detailed bit fields.
 *
 * This structure is used to construct a locomotive telegram for communication.
 * It contains the following fields:
 * - reserved (22 bits): Reserved for future use. [bits 0 - 21]
 * - stop_bit (1 bit): Stop bit of the telegram. Fixed value: 1. [bit 22]
 * - checksum (8 bits): An 8-bit field for the checksum value. Checksum is the XOR of the address and command bytes [bits 23 - 30]
 * - start_bit_cs (1 bit): Start bit for the checksum field. Fixed value: 0. [bit 31]
 * - speed (4 bits): Representing the speed of the locomotive.
 * - light (1 bit): Indicating whether the locomotive's light is on.
 * - direction (1 bit): Specifying the direction of the locomotive.
 * - cmd_bits_7_6 (2 bits): Bits 7 and 6 of the command block.
 * - start_bit_cmd (1 bit): Start bit for the command field. Fixed value: 0. [bit 40]
 * - address (7 bits): Representing the address of the locomotive.
 * - address_bit_7 (1 bit): The 7th bit of the address block.
 * - start_bit_address (1 bit): Start bit for the address field. Fixed value: 0. [bit 49]
 * - preamble (14 bits): Preamble for synchronization. Fixed value: 0b11111111111111 (0x3FFF). [bits 50 - 63]
 */
typedef struct
{
    unsigned long long reserved : 22;         // [bit 0 - 21]     Reserved for future use.
    unsigned long long stop_bit : 1;          // [bit 22]         Stop bit of the telegram. Fixed value: 1.
    unsigned long long checksum : 8;          // [bit 23 - 30]    An 8-bit field for the checksum value.
    unsigned long long start_bit_cs : 1;      // [bit 31]         Start bit for the checksum field. Fixed value: 0.
    unsigned long long speed : 4;             // [bit 32 - 35]    Speed of the locomotive. Values: 0 - 15 (0xF).
    unsigned long long light : 1;             // [bit 36]         Indicates if the light is on. Values: 0, 1.
    unsigned long long direction : 1;         // [bit 37]         Direction of the locomotive. Values: 0, 1.
    unsigned long long cmd_bits_7_6 : 2;      // [bit 38 - 39]    Command bits 7 and 6. Values: 0 - 3.
    unsigned long long start_bit_cmd : 1;     // [bit 40]         Start bit for the command field. Fixed value: 0.
    unsigned long long address : 7;           // [bit 41 - 47]    Address of the locomotive. Values: 0 - 127 (0x7F).
    unsigned long long address_bit_7 : 1;     // [bit 48]         7th bit of the address. Values: 0, 1.
    unsigned long long start_bit_address : 1; // [bit 49]         Start bit for the address field. Fixed value: 0.
    unsigned long long preamble : 14;         // [bit 50 - 63]    Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
} LocomotiveTelegram;

typedef union LocomotiveConverter
{
    LocomotiveTelegram locomotive_telegram;
    unsigned long long unsigned_long_long;
} LocomotiveConverter;

typedef union LocomotiveDataConverter
{
    LocomotiveData locomotive_data;
    unsigned short unsigned_short;
} LocomotiveDataConverter;

/**
 * @brief Constructs a LocomotiveTelegram from the given LocomotiveData.
 *
 * This function takes a LocomotiveData structure as input and constructs
 * a corresponding LocomotiveTelegram structure for communication.
 *
 * @param data The LocomotiveData structure containing the locomotive data.
 * @return A LocomotiveTelegram structure constructed from the input data.
 */
unsigned long long buildLocomotiveTelegram(LocomotiveData data);

#endif