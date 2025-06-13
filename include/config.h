#ifndef CONFIG_H
#define CONFIG_H

#include "telegram/locomotive.h"
#include "telegram/magnetic.h"

#define LOC_SIZE = 2
Locomotive locomotives_user[] = {
    {.alias = "loc3", .data = {
                          .address = 3,
                          .direction = 1,
                          .light = 1,
                          .speed = 0,
                      }},
    {.alias = "loc2", .data = {
                          .address = 2,
                          .direction = 1,
                          .light = 1,
                          .speed = 0,
                      }},
};

#define MAG_SIZE = 4
Magnetic magnetic_user[] = {
    {.alias = "switch0", .data = {
                             .address = 0,
                             .control = 0,
                             .device = 0,
                             .enable = 1,
                             .type = 0,
                             .ack = 0,
                         }},
    {.alias = "switch1", .data = {
                             .address = 0,
                             .control = 0,
                             .device = 1,
                             .enable = 1,
                             .type = 0,
                             .ack = 0,
                         }},
    {.alias = "switch2", .data = {
                             .address = 0,
                             .control = 0,
                             .device = 2,
                             .enable = 1,
                             .type = 0,
                             .ack = 0,
                         }},
    {.alias = "switch3", .data = {
                             .address = 0,
                             .control = 0,
                             .device = 3,
                             .enable = 1,
                             .type = 0,
                             .ack = 0,
                         }},
};

#endif