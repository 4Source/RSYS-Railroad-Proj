#include "communication/linux_rtai_communication.h"

#include <fcntl.h>
#include <unistd.h>

#define FIFO_CMD "dev/rtf3"
#define FIFO_ACK "dev/rtf4"

int send_with_ack(uint16_t data, int attempts = 3)
{
    // Open the command FIFO in write-only mode
    int fd_cmd = open(FIFO_CMD, O_WRONLY);
    if (fd_cmd < 0)
    {
        // If the command FIFO can't be opened, log the error
        printf("Failed to open command fifo!");
        return -1;
    }

    // Open the acknowledgment FIFO in read-only, non-blocking mode
    int fd_ack = open(FIFO_ACK, O_RDONLY | O_NONBLOCK);
    if (fd_ack < 0)
    {
        // If the acknowledgment FIFO can't be opened, log the error, close the command FIFO
        printf("Failed to open acknowledge fifo!");
        close(fd_cmd);
        return -1;
    }

    // Attempt to send the command multiple times (up to 'attempts')
    for (int attempt = 0; attempt < attempts; attempt++)
    {
        // Write the command data to the command FIFO
        write(fd_cmd, &data, sizeof(data));

        // Pause briefly (50 milliseconds) to wait for an acknowledgment
        usleep(50000);

        uint16_t ack;
        // Try reading the acknowledgment from the FIFO
        if (read(fd_ack, &ack, sizeof(ack)) == sizeof(ack))
        {
            // Check if the acknowledge received is from this command
            if ((0x7FFF & data) == (0x7FFF & ack))
            {
                // If the highest bit of ack is set, the command is acknowledged
                if ((0x8000 & ack) == 0x8000)
                {
                    // Close FIFOs
                    close(fd_cmd);
                    close(fd_ack);
                    return 0;
                }
            }
        }
    }

    // After all attempts, if no valid acknowledgment is received, log the failure
    printf("Failed to send command");
    // Close FIFOs
    close(fd_cmd);
    close(fd_ack);
    return -1;
}