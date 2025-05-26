#include "telegram/locomotive.h"

LocomotiveTelegram buildLocomotiveTelegram(LocomotiveData data)
{
    // Set the MSB to (0b0)
    uint8_t address = 0b00000000 | data.address;

    // Construct the command byte:
    // - The first two bits are set to 0b01
    // - The direction bit is shifted to bit 5
    // - The light bit is shifted to bit 4
    // - The speed value occupies bits 0 - 3
    uint8_t command = 0b01000000 | (data.direction << 5) | (data.light << 4) | data.speed;

    // Calculate the checksum as the XOR of the address and command bytes
    uint8_t checksum = address ^ command;

    // Build the LocomotiveTelegram structure with the calculated values
    LocomotiveTelegram telegram = {
        .preamble = 0b11111111111111, // Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
        .start_bit_address = 0,       // Start bit for the address section
        .address_bit_7 = 0,           // Reserved bit in the address section
        .address = data.address,      // Address of the locomotive
        .start_bit_cmd = 0,           // Start bit for the command section
        .cmd_bits_7_6 = 0b01,         // Fixed command bits
        .direction = data.direction,  // Direction of the locomotive
        .light = data.light,          // Light status (on/off)
        .speed = data.speed,          // Speed of the locomotive
        .start_bit_cs = 0,            // Start bit for the checksum section
        .checksum = checksum,         // Calculated checksum value.
        .stop_bit = 1,                // Stop bit
        .reserved = 0,                // Reserved field
    };

    return telegram;
}