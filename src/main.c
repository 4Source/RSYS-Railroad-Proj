#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"

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
