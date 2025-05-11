#include "reset.h"

ResetAllTelegram buildResetAllTelegram()
{
    ResetAllTelegram telegram = {
        .preamble = 0b11111111111111, // Preamble for synchronization. Fixed 14-bit value: 0b11111111111111 (0x3FFF).
        .start_bit_address = 0,       // Start bit for the address field. Fixed value: 0.
        .address = 0,                 // Address field. Fixed value: 0 (broadcast address).
        .start_bit_cmd = 0,           // Start bit for the command field. Fixed value: 0.
        .command = 0,                 // Command field. Fixed value: 0 (Reset command).
        .start_bit_cs = 0,            // Start bit for the checksum field. Fixed value: 0.
        .checksum = 0,                // Checksum value.
        .stop_bit = 1,                // Stop bit of the telegram. Fixed value: 1.
        .reserved = 0                 // Reserved field, set to 0
    };

    return telegram;
}