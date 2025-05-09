#ifndef MAGNETIC_H
#define MAGNETIC_H

#include <stdint.h>

/**
 * @struct MagneticData
 * @brief Represents the magnetic data with bit fields for various parameters.
 *
 * This structure contains the following fields:
 * - enable: A 1-bit field indicating whether the magnetic device is enabled (0 or 1).
 * - device: A 2-bit field specifying the device type (0 to 3).
 * - control: A 1-bit field for control status (0 or 1).
 * - address: A 9-bit field representing the address (0 to 511).
 * - reserved: A 3-bit reserved field for future use.
 */
typedef struct
{
    uint16_t enable : 1;   // [bit 0]        Enables or disables the magnetic device. Values: 0, 1.
    uint16_t device : 2;   // [bit 1 - 2]    Device type identifier. Values: 0 - 3.
    uint16_t control : 1;  // [bit 3]        Control status of the device. Values: 0, 1.
    uint16_t address : 9;  // [bit 4 - 12]   9-bit address of the device. Values: 0 - 511 (0x1FF).
    uint16_t reserved : 3; // [bit 13 - 15]  Reserved for future use.
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
    const char *alias; // A constant character pointer for the alias name.
} Magnetic;

/**
 * @struct MagneticTelegram
 * @brief Represents a magnetic telegram with detailed bit fields.
 *
 * This structure is used to construct a magnetic telegram for communication.
 * It contains the following fields:
 * - reserved: A 22-bit reserved field for future use.
 * - stop_bit: A 1-bit field indicating the stop_bit of the telegram.
 * - checksum: An 8-bit field for the checksum value.
 * - start_bit_cs: A 1-bit field for the start bit of the checksum.
 * - enable: A 1-bit field indicating whether the magnetic device is enabled.
 * - device: A 2-bit field specifying the device type.
 * - control: A 1-bit field for control status.
 * - address_high: A 3-bit field for the high bits of the address.
 * - cmd_bit_7: A 1-bit field for the 7th bit of the command.
 * - start_bit_cmd: A 1-bit field for the start bit of the command.
 * - address_low: A 6-bit field for the low bits of the address.
 * - address_bits_7_6: A 2-bit field for bits 7 and 6 of the address.
 * - start_bit_address: A 1-bit field for the start bit of the address.
 * - preamble: A 14-bit field for the preamble.
 */
typedef struct
{
    uint64_t reserved : 22;         // [bit 0 - 21]    Reserved for future use.
    uint64_t stop_bit : 1;          // [bit 22]        Indicates the stop_bit of the telegram. Values: 0, 1.
    uint64_t checksum : 8;          // [bit 23 - 30]   Checksum value for error detection. Values: 0 - 255 (0xFF).
    uint64_t start_bit_cs : 1;      // [bit 31]        Start bit for checksum field. Values: 0, 1.
    uint64_t enable : 1;            // [bit 32]        Enables or disables the magnetic device. Values: 0, 1.
    uint64_t device : 2;            // [bit 33 - 34]   Device type identifier. Values: 0 - 3.
    uint64_t control : 1;           // [bit 35]        Control status of the device. Values: 0, 1.
    uint64_t address_high : 3;      // [bit 36 - 38]   High 3 bits of the 9-bit address. Values: 0 - 7.
    uint64_t cmd_bit_7 : 1;         // [bit 39]        7th bit of the command field. Values: 0, 1.
    uint64_t start_bit_cmd : 1;     // [bit 40]        Start bit for the command field. Values: 0, 1.
    uint64_t address_low : 6;       // [bit 41 - 46]   Low 6 bits of the 9-bit address. Values: 0 - 63 (0x3F).
    uint64_t address_bits_7_6 : 2;  // [bit 47 - 48]   Bits 7 and 6 of the 9-bit address. Values: 0 - 3.
    uint64_t start_bit_address : 1; // [bit 49]        Start bit for the address field. Values: 0, 1.
    uint64_t preamble : 14;         // [bit 50 - 63]   Preamble for synchronization. Values: 0 - 16383 (0x3FFF).
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
MagneticTelegram buildMagneticTelegram(MagneticData data);

#endif