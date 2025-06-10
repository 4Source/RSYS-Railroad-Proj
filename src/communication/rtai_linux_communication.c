#include "communication/rtai_linux_communication.h"

#include <rtai_fifos.h>

#include "communication/railroad_communication.h"

#define STACK_SIZE 4096


int fifo_handler(unsigned int fifo)
{
    char command[1024];
    int r;
    unsigned short raw;

    r = rtf_get(fifo, command, sizeof(command) - 1);
    if (r >= sizeof(unsigned short))
    {
        memcpy(&raw, command, sizeof(unsigned short));

        // Typ prüfen (bitweise: Bit 13-14)
        unsigned short type = (raw >> 13) & 0x3;

        if (type == 0x1)
        { // Locomotive
            LocomotiveData loco = *(LocomotiveData *)&raw;

            if (loco.address < 4 && loco.address >= 0)
            {
                rt_mutex_lock(&loc_sem[loco.address - 1]);
                locomotive_msg_queue[loco.address - 1] = loco;
                rt_mutex_unlock(&loc_sem[loco.address - 1]);
                printk("Locomotive Addr %d: Speed=%d Dir=%d Light=%d\n", loco.address, loco.speed, loco.direction, loco.light);
                send_ack(raw);
            }
            else
            {
                printk("Ungültige Lok-Adresse: %d\n", loco.address);
            }
        }
        else if (type == 0x2)
        { // Magnetic
            MagneticData mag = *(MagneticData *)&raw;

            if (magnetic_msg_count < 4)
            {
                rt_mutex_lock(&mag_sem[magnetic_msg_count]);
                magnetic_msg_queue[magnetic_msg_count] = mag;
                rt_mutex_unlock(&mag_sem[magnetic_msg_count]);
                magnetic_msg_count++;
                printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n", mag.address, mag.device, mag.enable, mag.control);
                send_ack(raw); 
                if (magnetic_task == NULL)
                {
                    rt_task_init(magnetic_task, send_magnetic_msg_task, 0, STACK_SIZE, 1, 0, 0);
                }
            }
            else
            {
                printk("Magnetic queue voll!\n");
            }
        }
        else
        {
            printk("Unbekannter Nachrichtentyp: %d\n", type);
        }
    }
    else
    {
        printk("Ungültige FIFO-Daten (nur %d Byte)\n", r);
    }

    return 0;
}

void send_ack(unsigned short raw)
{
    // Setze Bit 15 (ACK-Bit)
    raw |= (1 << 15);

    int result = rtf_put(FIFO_ACK, &raw, sizeof(raw));

    if (result != sizeof(raw))
    {
        printk("ACK konnte nicht gesendet werden.\n");
    }
}
