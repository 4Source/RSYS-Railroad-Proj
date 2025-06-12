#include "communication/rtai_linux_communication.h"
#include "communication/railroad_communication.h"

void send_ack(unsigned short raw)
{
    // Set bit 15 as the ACK flag.
    raw |= (1 << 15);

    int result = rtf_put(FIFO_ACK, &raw, sizeof(raw));

    if (result < 0)
    {
        rt_printk("Error sending ACK: rtf_put returned %d.\n", result);
    }
    else if (result != sizeof(raw))
    {
        rt_printk("ACK could not be sent completely. Expected: %lu, received: %d.\n", sizeof(raw), result);
    }
}

int findIndexOfLocAddress(LocomotiveData loc)
{
    int i;
    for (i = 0; i < LOC_MSQ_SIZE; i++)
    {
        if (locomotive_msg_queue[i].address == loc.address)
        {
            return i;
        }
    }
    return -1;
}

int findIndexOfMagAddress(MagneticData mag)
{
    int i;
    for (i = 0; i < MAG_MSQ_SIZE; i++)
    {
        if (magnetic_msg_queue[i].address == mag.address && magnetic_msg_queue[i].device == mag.device)
        {
            return i;
        }
    }
    return -1;
}

int fifo_handler(unsigned int fifo)
{
    char command[FIFO_SIZE];
    int r;
    unsigned short raw;

    // Get data from FIFO_CMD channel.
    r = rtf_get(FIFO_CMD, command, sizeof(command) - 1);
    rt_printk("Fifo handler received %d bytes\n", r);

    if (r >= sizeof(unsigned short))
    {
        // Extract the 2-byte raw command
        memcpy(&raw, command, sizeof(unsigned short));
        rt_printk("Fifo handler received command 0x%04X\n", raw);

        // Extract command type from bits 13-14. (Shift and mask with 0x3)
        unsigned short type = (raw >> 13) & 0x3;

        if (type == 0x1) // Locomotive command
        {
            rt_printk("Command is of type: locomotive");
            // Use union conversion to obtain message structure from raw
            LocomotiveDataConverter converter = {.unsigned_short = raw};

            // Find correct index in locomotive_msg_queue
            int index = findIndexOfLocAddress(converter.locomotive_data);
            rt_printk("Index of address %d is %d", converter.locomotive_data.address, index);
            if (index >= 0)
            {
                // Protect access to message queue using semaphore
                rt_sem_wait(&loc_sem[index]);
                locomotive_msg_queue[index] = converter.locomotive_data;
                rt_sem_signal(&loc_sem[index]);
                rt_printk("Locomotive Addr %d: Speed=%d Dir=%d Light=%d\n", converter.locomotive_data.address, converter.locomotive_data.speed, converter.locomotive_data.direction, converter.locomotive_data.light);
                send_ack(raw);
            }
            else
            {
                rt_printk("Invalid locomotive address: %d\n", converter.locomotive_data.address);
            }
        }
        else if (type == 0x2) // Magnetic command
        {
            rt_printk("Command is of type: magnetic");
            MagneticDataConverter converter = {.unsigned_short = raw};

            // Find matching message in magnetic queue
            int index = findIndexOfMagAddress(converter.magnetic_data);
            rt_printk("Index of address %d device %d is %d", converter.magnetic_data.address, converter.magnetic_data.device, index);
            if (index >= 0)
            {
                rt_sem_wait(&mag_sem[index]);
                magnetic_msg_queue[index] = converter.magnetic_data;
                rt_sem_signal(&mag_sem[index]);
                rt_printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n", converter.magnetic_data.address, converter.magnetic_data.device, converter.magnetic_data.enable, converter.magnetic_data.control);
                send_ack(raw);
            }
            else if (magnetic_msg_count < MAG_MSQ_SIZE)
            {
                // If not found, use next free slot
                rt_sem_wait(&mag_sem[magnetic_msg_count]);
                magnetic_msg_queue[magnetic_msg_count] = converter.magnetic_data;
                rt_sem_signal(&mag_sem[magnetic_msg_count]);
                magnetic_msg_count++;
                rt_printk("Magnetic Addr %d: Device=%d Enable=%d Ctrl=%d\n", converter.magnetic_data.address, converter.magnetic_data.device, converter.magnetic_data.enable, converter.magnetic_data.control);
                send_ack(raw);
            }
            else
            {
                rt_printk("Magnetic queue full!\n");
            }
        }
        else
        {
            rt_printk("Unknown message type: %d\n", type);
        }
    }
    else
    {
        rt_printk("Invalid FIFO data (only %d bytes)\n", r);
    }

    return 0;
}

// EXPORT_SYMBOL(fifo_handler);
