#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "magnetic.h"
#include "locomotive.h"
#include "command.h"

Locomotive locomotives[] = {{.alias = "loc3",
                             .data = {
                                 .address = 3,
                                 .direction = 1,
                                 .light = 1,
                                 .speed = 0,
                             }}};

int main()
{
    int exit;

    do
    {
        char *command = NULL;
        char *args = NULL;

        if (prompt(&command, &args) != 0)
        {
            continue;
        }

        exit = handle_command(command, args);

        free(command);
        free(args);
    } while (exit == 1);

    return exit;
}
