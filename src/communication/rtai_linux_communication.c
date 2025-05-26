#include "rtai_linux_communication.h"
#include "railroad_communication.h"
#include <string.h>
#include <stdint.h>
 
// größe array speichern und belegte größe
// auf letzte freie zeigen (wenn index = 0 dann leer)
// Funktion von Fifo einlesen und in queue speichern (Übung 5 fifo handler)
// bitmaske für adressen, dementsprechen in locomotive oder magnetic queue legen
//  locomotive queue hat vorher festgelegte indizes für die jeweilige adresse

int fifo_handler(unsigned int fifo)
{
    char command[1024];
    int r;
    uint16_t raw;

    r = rtf_get(fifo, command, sizeof(command) - 1);
    if (r >= sizeof(uint16_t)) {
        memcpy(&raw, command, sizeof(uint16_t));

        // Typ prüfen (bitweise: Bit 13-14)
        uint16_t type = (raw >> 13) & 0x3;

        if (type == 0x1) {  // Locomotive
            LocomotiveData loco = *(LocomotiveData*)&raw;

            if (loco.address < 4 && loco.address >= 0) {
                locomotive_msg_queue[loco.address-1] = loco;
                printk("Locomotive Addr %d: Speed=%d Dir=%d Light=%d\n",
                       loco.address, loco.speed, loco.direction, loco.light);
            } else {
                printk("Ungültige Lok-Adresse: %d\n", loco.address);
            }

        } else if (type == 0x2) {  // Magnetic
            MagneticData mag = *(MagneticData*)&raw;

            if (magnetic_msg_count < 4) {
                magnetic_msg_queue[magnetic_msg_count++].command = raw;
                printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n",
                       mag.address, mag.device, mag.enable, mag.control);
                    if (!rt_task_alive(&magnetic_task)) {
                        rt_task_init(&magnetic_task, send_magnetic_msg_task, 0, STACK_SIZE, 1, 0, 0);
                    }
            rt_task_resume(&magnetic_task);
            } else {
                printk("Magnetic queue voll!\n");
            }
        } else {
            printk("Unbekannter Nachrichtentyp: %d\n", type);
        }

        // Optional: ACK senden
        // uint8_t ack = 0xA5;
        // rtf_put(ACK_FIFO, &ack, sizeof(ack));

    } else {
        printk("Ungültige FIFO-Daten (nur %d Byte)\n", r);
    }

    return 0;
}
