#ifndef MAGNETIC_H
#define MAGNETIC_H

/**
 * @struct MagneticData
 * @brief Represents the magnetic data with bit fields for various parameters.
 *
 * This structure contains the following fields:
 * - enable (1 bit): Indicating whether the magnetic device is enabled (0 or 1).
 * - device (2 bits): Specifying the device type (0 to 3).
 * - control (1 bit): Control status (0 or 1).
 * - address (9 bits): Representing the address (0 to 511).
 * - type (2 bits): Type of data (01 = Locomotive, 10 = Magnetic).
 * - ack (1 bit): Acknowledge flag (0 = not acknowledged, 1 = acknowledged).
 */
typedef struct
{
    unsigned short enable : 1;  // [bit 0]        Enables or disables the magnetic device. Values: 0 (off), 1 (on).
    unsigned short device : 2;  // [bit 1 - 2]    Device type identifier. Values: 0 - 3 (devices 1 - 4).
    unsigned short control : 1; // [bit 3]        Control status of the device. Values: 0 (off), 1 (on).
    unsigned short address : 9; // [bit 4 - 12]   9-bit address of the device. Values: 0 - 511 (0x1FF).
    unsigned short type : 2;    // [bit 13 - 14]  Type of data: 01 (Locomotive), 10 (Magnetic)
    unsigned short ack : 1;     // [bit 15]       Acknowledge: 0 (not), 1 (ack)
} MagneticData;

/**
 * @struct Magnetic
 * @brief Combines magnetic data with an alias for identification.
 *
 * This structure contains:
 * - data: A MagneticData structure holding the magnetic data.
 * - alias: A constant character pointer for the alias name.
 */
typedef struct
{
    MagneticData data; // A MagneticData structure holding the magnetic data.
    char alias[20];    // A character array with a fixed length of 20 for the alias name.
} Magnetic;

/**
 * @struct MagneticTelegram
 * @brief Represents a magnetic telegram with detailed bit fields.
 *
 * This structure is used to construct a magnetic telegram for communication.
 * It contains the following fields:
 * - reserved (22 bits): Reserved for future use. [bits 0 - 21]
 * - stop_bit (1 bit): Stop bit of the telegram. Fixed value: 1. [bit 22]
 * - checksum (8 bits): An 8-bit field for the checksum value. Checksum is the XOR of the address and command bytes [bits 23 - 30]
 * - start_bit_cs (1 bit): Start bit for the checksum field. Fixed value: 0. [bit 31]
 * - enable (1 bit): Indicating whether the magnetic device is enabled.
 * - device (2 bits): Specifying the device type.
 * - control (1 bit): Control status.
 * - address_high (3 bits): The high bits of the address.
 * - cmd_bit_7 (1 bit): The 7th bit of the command block.
 * - start_bit_cmd (1 bit): Start bit for the command field. Fixed value: 0. [bit 40]
 * - address_low (6 bits): The low bits of the address.
 * - address_bits_7_6 (2 bits): Bits 7 and 6 of the address block.
 * - start_bit_address (1 bit): Start bit for the address field. Fixed value: 0. [bit 49]
 * - preamble (14 bits): Preamble for synchronization. Fixed value: 0b11111111111111 (0x3FFF). [bits 50 - 63]
 */
typedef struct
{
    unsigned long long reserved : 22;         // [bit 0 - 21]     Reserved for future use.
    unsigned long long stop_bit : 1;          // [bit 22]         Stop bit of the telegram. Fixed value: 1.
    unsigned long long checksum : 8;          // [bit 23 - 30]    An 8-bit field for the checksum value.
    unsigned long long start_bit_cs : 1;      // [bit 31]         Start bit for the checksum field. Fixed value: 0.
    unsigned long long enable : 1;            // [bit 32]         Enables or disables the magnetic device. Values: 0, 1.
    unsigned long long device : 2;            // [bit 33 - 34]    Device type identifier. Values: 0 - 3.
    unsigned long long control : 1;           // [bit 35]         Control status of the device. Values: 0, 1.
    unsigned long long address_high : 3;      // [bit 36 - 38]    High 3 bits of the 9-bit address. Values: 0 - 7.
    unsigned long long cmd_bit_7 : 1;         // [bit 39]         7th bit of the command field. Values: 0, 1.
    unsigned long long start_bit_cmd : 1;     // [bit 40]         Start bit for the command field. Fixed value: 0.
    unsigned long long address_low : 6;       // [bit 41 - 46]    Low 6 bits of the 9-bit address. Values: 0 - 63 (0x3F).
    unsigned long long address_bits_7_6 : 2;  // [bit 47 - 48]    bits 7 and 6 of the 9-bit address. Values: 0 - 3.
    unsigned long long start_bit_address : 1; // [bit 49]         Start bit for the address field. Fixed value: 0.
    unsigned long long preamble : 14;         // [bit 50 - 63]    Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
} MagneticTelegram;

/**
 * @brief Constructs a MagneticTelegram from the given MagneticData.
 *
 * This function takes a MagneticData structure as input and constructs
 * a corresponding MagneticTelegram structure for communication.
 *
 * @param data The MagneticData structure containing the magnetic data.
 * @return A MagneticTelegram structure constructed from the input data.
 */
unsigned long long buildMagneticTelegram(MagneticData data);

typedef union MagneticConverter
{
    MagneticTelegram mt;
    unsigned long long ull;
} MagneticConverter;

typedef union MagneticDataConverter
{
    MagneticData md;
    unsigned short us;
} MagneticDataConverter;

#endif