#include "magnetic.h"

MagneticTelegram buildMagneticTelegram(MagneticData data)
{
    // Extract the high bits of the address and invert them (bits 8-6 of the address)
    uint16_t address_high = ~((data.address >> 6) & 0b111);

    // Extract the low bits of the address (bits 5-0 of the address)
    uint8_t address_low = (data.address & 0b111111);

    // Construct the address byte:
    // - The MSB is set to 0b10
    // - The lower 6 bits are taken from the low bits of the address
    uint8_t address = 0b10000000 | address_low;

    // Construct the command byte:
    // - The MSB is set to 0b1
    // - The high bits of the address (inverted) are shifted to bits 4-6
    // - The control bit is shifted to bit 3
    // - The device type occupies bit 2-1
    // - The enable bit occupies bit 0
    uint8_t command = 0b10000000 | (address_high << 4) | (data.control << 3) | (data.device << 1) | data.enable;

    // Calculate the checksum as the XOR of the address and command bytes
    uint8_t checksum = address ^ command;

    // Build the MagneticTelegram structure with the calculated and fixed values
    MagneticTelegram telegram = {
        .preamble = 0b11111111111111, // Fixed 14-bit preamble value
        .start_bit_address = 0,       // Start bit for the address field
        .address_bits_7_6 = 0b10,     // Fixed high bits (7-6) of the address
        .address_low = address_low,   // Low bits (5-0) of the address
        .start_bit_cmd = 0,           // Start bit for the command field
        .cmd_bit_7 = 1,               // Fixed high bit (7) of the command
        .address_high = address_high, // High bits (8-6) of the address
        .control = data.control,      // Control field from the input data
        .device = data.device,        // Device field from the input data
        .enable = data.enable,        // Enable field from the input data
        .start_bit_cs = 0,            // Start bit for the checksum field
        .checksum = checksum,         // Calculated checksum value
        .stop_bit = 1,                // Fixed stop bit
        .reserved = 0                 // Reserved field, set to 0
    };

    return telegram;
}