#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"
#include "config.h"
#include "communication/linux_rtai_communication.h"

#define CMD_CNT 5
Command commands[] = {
    {"loc", cmd_loc, "Usage: loc (--address <address> | --alias <alias>) [OPTION]...\n", "Description: Gives access to the configuration for locomotives.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the locomotive which should be changed. Address range is 0 to 127.\n  -A <alias>, --alias <alias>                                                  Select the alias of the locomotive which should be changed. Is internally resolved to the address which is configured for this alias. The length of alias is limited to 20 characters.\n  -d (forward | backward), --direction (forward | backward)                    Set the direction in which the locomotive should drive.\n  -h, --help                                                                   Show this screen.\n  -l (on|off), --light (on|off)                                                Enable or disable the light of the locomotive.\n  --list                                                                       List the available locomotives.\n  -m, --monitor                                                                Shows the current configuration of the locomotive.\n  -s <stop | e-stop | 0-15>, --speed <stop | e-stop | 0-15>                    Set the speed the locomotive should drive.\n"},
    {"mag", cmd_mag, "Usage: mag (--address <address> --device <device> | --alias <alias>) [OPTION]...\n", "Description: Gives access to the configuration for magnetic accessories.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the accessory which should be changed. Address range is 0 to 511.\n  -A <alias>, --alias <alias>                                                  Select the alias of the accessory which should be changed. Is internally resolved to the address which is configured for this alias. The length of alias is limited to 20 characters.\n  -d <device>, --device <device>                                               Select the device (1-4) which which should be changed.\n  --list                                                                       List the available magnetics.\n  -m, --monitor                                                                Shows the current configuration of the magnetic.\n  -s (on|off), --switch (on|off)                                               Enable or disable the switch.\n"},
    {"restore", cmd_restore, "Usage: restore [digital]\n", "Description: Restores the digital mode. Use this command if the system has switched to an alternative mode (analog mode).\n", ""},
    {"help", cmd_help, "Usage: help\n       help <command>\n", "Description: Show this help or used with <command> --help.\n", ""},
    {"exit", NULL, "Usage: exit\n", "Description: Terminates the current prompt. Also sends a reset message to all decoders.\n", ""},
    {NULL, NULL, NULL}};

/**
 * @brief Reads user input and parses it into a command and arguments.
 *
 * @param input Buffer to store the user input.
 * @param command Pointer to store the parsed command.
 * @param args Pointer to store the parsed arguments.
 * @return int Return codes:
 *         0 - Success, input parsed successfully.
 *         1 - End of input (e.g., EOF).
 *         2 - Error: No command provided.
 */
int prompt(char **command, char **args)
{
    char input[MAX_INPUT];

    printf("dcc> ");
    fflush(stdout);

    if (!fgets(input, MAX_INPUT, stdin))
    {
        printf("\n");
        // End of input (e.g., EOF)
        return 1;
    }

    // Remove trailing newline
    input[strcspn(input, "\n")] = 0;

    free(*command);
    *command = NULL;
    free(*args);
    *args = NULL;

    // Tokenize input and create copies of tokens
    char *token = strtok(input, " ");
    if (token)
    {
        *command = strdup(token);
        token = strtok(NULL, "\0");
        *args = token ? strdup(token) : NULL;
    }

    if (!*command || !**command)
    {
        // Error: No command provided
        return 2;
    }

    return 0;
}

int handle_command(const char *command, char *args)
{
    // Look up command
    int found = 0;
    for (int i = 0; i < CMD_CNT; i++)
    {
        if (commands[i].name && strcmp(command, commands[i].name) == 0)
        {
            found = 1;

            // Support --help as argument
            if (args && (strcmp(args, "--help") == 0 || strcmp(args, "-h") == 0))
            {
                printf(commands[i].usage);
                if (commands[i].description != "")
                {
                    printf("\n");
                }
                printf(commands[i].description);
                if (commands[i].options != "")
                {
                    printf("\n");
                }
                printf(commands[i].options);
            }
            else if (commands[i].func)
            {
                commands[i].func(args);
            }
            else
            {
                return 0;
            }
            break;
        }
    }

    if (!found)
    {
        printf("Unknown command: %s\n", command);
    }
    return 1;
}

void cmd_loc(char *args)
{
    const char *cmd_name = "loc";
    int options_valid = 1;

    int address = -1;
    char alias[20] = "";
    int direction = -1;
    int light = -1;
    int speed = -1;
    int monitor = 0;

    // Tokenize the arguments
    char *option = strtok(args, " ");
    while (option != NULL)
    {
        if (strcmp(option, "-a") == 0 || strcmp(option, "--address") == 0)
        {
            // Get the value for the address
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                if (sscanf(value, "%d", &address) != 1 || address > 127)
                {
                    printf("Invalid argument '%s' for address\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for address\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-A") == 0 || strcmp(option, "--alias") == 0)
        {
            // Get the value for the alias
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                // Only allow alphanumeric aliases
                if (sscanf(value, "%[A-Z,a-z,0-9]s", &alias) != 1)
                {
                    printf("Invalid argument '%s' for alias\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for alias\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-d") == 0 || strcmp(option, "--direction") == 0)
        {
            // Get the value for the direction
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                if (strcmp(value, "forward") == 0)
                {
                    direction = 1;
                }
                else if (strcmp(value, "backward") == 0)
                {
                    direction = 0;
                }
                else
                {
                    printf("Invalid argument '%s' for direction\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for direction\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-l") == 0 || strcmp(option, "--light") == 0)
        {
            // Get the value for the light
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                if (strcmp(value, "on") == 0)
                {
                    light = 1;
                }
                else if (strcmp(value, "off") == 0)
                {
                    light = 0;
                }
                else
                {
                    printf("Invalid argument '%s' for light\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for light\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-s") == 0 || strcmp(option, "--speed") == 0)
        {
            // Get the value for the speed
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                if (strcmp(value, "stop") == 0)
                {
                    speed = 0;
                }
                else if (strcmp(value, "e-stop") == 0)
                {
                    speed = 1;
                }
                else if (sscanf(value, "%d", &speed) != 1 || speed > 15)
                {
                    printf("Invalid argument '%s' for speed\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for speed\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-m") == 0 || strcmp(option, "--monitor") == 0)
        {
            monitor = 1;
        }
        else if (strcmp(option, "--list") == 0)
        {
            // List the available locomotives
            size_t num_locomotives = sizeof(locomotives_user) / sizeof(locomotives_user[0]);
            printf("Locomotives:\n");
            for (size_t i = 0; i < num_locomotives; i++)
            {
                printf("\taddress: %d (%s) - direction: %d, light: %d, speed: %d\n", locomotives_user[i].data.address, locomotives_user[i].alias, locomotives_user[i].data.direction, locomotives_user[i].data.light, locomotives_user[i].data.speed);
            }
            return;
        }
        else
        {
            printf("Unknown option '%s' for command '%s'\n", option, cmd_name);
            options_valid = 0;
        }

        // Move to the next option
        option = strtok(NULL, " ");
    }

    if (!options_valid)
    {
        printf("See '%s --help' for more informations.\n", cmd_name);
        return;
    }

    // Check if address or alias is set
    if (address >= 0 || alias[0] != '\0')
    {
        // Prepare a temporary Locomotive struct to hold the updated values
        Locomotive loc = {
            .alias = "",
            .data = {
                .address = 0,
                .direction = 0,
                .light = 0,
                .speed = 0,
                .type = 0,
                .ack = 0,
            }};

        size_t num_locomotives = sizeof(locomotives_user) / sizeof(locomotives_user[0]);
        for (int i = 0; i < num_locomotives; i++)
        {
            // Find the matching locomotive by address or alias
            if (address == locomotives_user[i].data.address || (alias[0] != '\0' && strcmp(alias, locomotives_user[i].alias) == 0))
            {
                // If alias was not set, copy from found locomotive
                if (alias[0] == '\0')
                {
                    strcpy(loc.alias, locomotives_user[i].alias);
                }
                else
                {
                    strcpy(loc.alias, alias);
                }
                // If address was not set, copy from found locomotive
                if (address < 0)
                {
                    loc.data.address = locomotives_user[i].data.address;
                }
                else
                {
                    loc.data.address = address;
                }
                // If direction were not set, use existing values, otherwise update
                if (direction < 0)
                {
                    loc.data.direction = locomotives_user[i].data.direction;
                }
                else
                {
                    loc.data.direction = direction;
                    locomotives_user[i].data.direction = direction;
                }
                // If light were not set, use existing values, otherwise update
                if (light < 0)
                {
                    loc.data.light = locomotives_user[i].data.light;
                }
                else
                {
                    loc.data.light = light;
                    locomotives_user[i].data.light = light;
                }
                // If speed were not set, use existing values, otherwise update
                if (speed < 0)
                {
                    loc.data.speed = locomotives_user[i].data.speed;
                }
                else
                {
                    loc.data.speed = speed;
                    locomotives_user[i].data.speed = speed;
                }
                break;
            }
        }

        if (monitor)
        {
            printf("address: %d (%s) - direction: %d, light: %d, speed: %d\n", loc.data.address, loc.alias, loc.data.direction, loc.data.light, loc.data.speed);
        }

        // TODO: Send changes to rtai part and check for acknowledge. Measure time between send and checks. Check the timeout exceeded. Check the FIFO contains the message if not but no acknowledge resend.
        // Send not changes instead send unsigned short rtai side needs to check if there is a matching object to the address
        send_with_ack(loc.data, 3);
    }
    else
    {
        // If neither address nor alias is set, print usage
        for (int i = 0; i < CMD_CNT; i++)
        {
            if (strcmp(cmd_name, commands[i].name) == 0)
            {
                printf(commands[i].usage);
                return;
            }
        }
    }
}

void cmd_mag(char *args)
{
    const char *cmd_name = "mag";
    int options_valid = 1;

    int address = -1;
    char alias[20] = "";
    int device = -1;
    int control = -1;
    int monitor = 0;

    // Tokenize the arguments
    char *option = strtok(args, " ");
    while (option != NULL)
    {
        if (strcmp(option, "-a") == 0 || strcmp(option, "--address") == 0)
        {
            // Get the value for the address
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                if (sscanf(value, "%d", &address) != 1 || address > 512)
                {
                    printf("Invalid argument '%s' for address\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for address\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-A") == 0 || strcmp(option, "--alias") == 0)
        {
            // Get the value for the alias
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                // Only allow alphanumeric aliases
                if (sscanf(value, "%[A-Z,a-z,0-9]s", &alias) != 1)
                {
                    printf("Invalid argument '%s' for alias\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for alias\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-d") == 0 || strcmp(option, "--device") == 0)
        {
            // Get the value for the device
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                // Device must be in range 1-4, but is stored as 0-3
                if (sscanf(value, "%d", &device) != 1 || device > 4 || device < 1)
                {
                    printf("Invalid argument '%s' for device\n", value);
                    options_valid = 0;
                }
                device -= 1;
            }
            else
            {
                printf("Missing argument for device\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-s") == 0 || strcmp(option, "--switch") == 0)
        {
            // Get the value for the switch
            char *value = strtok(NULL, " ");
            if (value != NULL)
            {
                if (strcmp(value, "on") == 0)
                {
                    control = 1;
                }
                else if (strcmp(value, "off") == 0)
                {
                    control = 0;
                }
                else
                {
                    printf("Invalid argument '%s' for switch\n", value);
                    options_valid = 0;
                }
            }
            else
            {
                printf("Missing argument for switch\n");
                options_valid = 0;
            }
        }
        else if (strcmp(option, "-m") == 0 || strcmp(option, "--monitor") == 0)
        {
            monitor = 1;
        }
        else if (strcmp(option, "--list") == 0)
        {
            // List the available magnetics
            size_t num_magnetics = sizeof(magnetic_user) / sizeof(magnetic_user[0]);
            printf("Magnetics:\n");
            for (size_t i = 0; i < num_magnetics; i++)
            {
                printf("\taddress: %d (%s) - device: %d, control: %d, enable: %d\n", magnetic_user[i].data.address, magnetic_user[i].alias, magnetic_user[i].data.device, magnetic_user[i].data.control, magnetic_user[i].data.enable);
            }
            return;
        }
        else
        {
            printf("Unknown option '%s' for command '%s'\n", option, cmd_name);
            options_valid = 0;
        }

        // Move to the next option
        option = strtok(NULL, " ");
    }

    if (!options_valid)
    {
        printf("See '%s --help' for more informations.\n", cmd_name);
        return;
    }

    // Check if address/device or alias is set
    if ((address >= 0 && device >= 0) || alias[0] != '\0')
    {
        // Prepare a temporary Magnetic struct to hold the updated values
        Magnetic mag = {
            .alias = "",
            .data = {
                .address = 0,
                .control = 0,
                .device = 0,
                .enable = 0,
                .type = 0,
                .ack = 0,
            }};

        size_t num_magnetics = sizeof(magnetic_user) / sizeof(magnetic_user[0]);
        for (int i = 0; i < num_magnetics; i++)
        {
            // Find the matching magnetic by address or alias
            if (address == magnetic_user[i].data.address || (alias[0] != '\0' && strcmp(alias, magnetic_user[i].alias) == 0))
            {
                // If alias was not set, copy from found magnetic
                if (alias[0] == '\0')
                {
                    strcpy(mag.alias, magnetic_user[i].alias);
                }
                else
                {
                    strcpy(mag.alias, alias);
                }
                // If address was not set, copy from found magnetic
                if (address < 0)
                {
                    mag.data.address = magnetic_user[i].data.address;
                }
                else
                {
                    mag.data.address = address;
                }
                // If control were not set, use existing values, otherwise update
                if (control < 0)
                {
                    mag.data.control = magnetic_user[i].data.control;
                }
                else
                {
                    mag.data.control = control;
                    magnetic_user[i].data.control = control;
                }
                // If device were not set, use existing values, otherwise update
                if (device < 0)
                {
                    mag.data.device = magnetic_user[i].data.device;
                }
                else
                {
                    mag.data.device = device;
                    magnetic_user[i].data.device = device;
                }

                mag.data.enable = magnetic_user[i].data.enable;
                break;
            }
        }

        if (monitor)
        {
            printf("address: %d (%s) - device: %d, control: %d, enable: %d\n", mag.data.address, mag.alias, mag.data.device, mag.data.control, mag.data.enable);
        }

        // TODO: Send changes to rtai part and check for acknowledge. Measure time between send and checks. Check the timeout exceeded. Check the FIFO contains the message if not but no acknowledge resend.
        send_with_ack(mag.data, 3);
    }
    else
    {
        // If neither address/device nor alias is set, print usage
        for (int i = 0; i < CMD_CNT; i++)
        {
            if (strcmp(cmd_name, commands[i].name) == 0)
            {
                printf(commands[i].usage);
                return;
            }
        }
    }
}

void cmd_restore(char *args)
{
    const char *cmd_name = "restore";
    int options_valid = 1;

    enum power_mode mode = digital;

    // Tokenize the arguments
    char *option = strtok(args, " ");
    while (option != NULL)
    {
        if (strcmp(option, "digital") == 0)
        {
            mode = digital;
        }
        else if (strcmp(option, "analog") == 0)
        {
            mode = analog;
        }
        else
        {
            printf("Unknown option '%s' for command '%s'\n", option, cmd_name);
            options_valid = 0;
        }

        // Move to the next option
        option = strtok(NULL, " ");
    }

    if (!options_valid)
    {
        printf("See '%s --help' for more informations.\n", cmd_name);
        return;
    }

    switch (mode)
    {
    case digital:
        printf("Restore mode: digital\n");
        // TODO: Call corrsponding function
        break;
    case analog:
        printf("Currently unsupported to switch to mode 'analog'!");
        break;

    default:
        for (int i = 0; i < CMD_CNT; i++)
        {
            if (strcmp(cmd_name, commands[i].name) == 0)
            {
                printf(commands[i].usage);
                return;
            }
        }
        break;
    }
}

void cmd_help(char *args)
{
    if (args && strlen(args) > 0)
    {
        // Check if help is requested for a specific command
        for (int i = 0; i < CMD_CNT; i++)
        {
            if (strcmp(args, commands[i].name) == 0)
            {
                printf(commands[i].usage);
                return;
            }
        }
        printf("No such command: %s\n", args);
    }
    else
    {
        printf("Available commands:\n");
        for (int i = 0; i < CMD_CNT; i++)
        {
            printf(commands[i].usage);
            printf(commands[i].description);
            printf("\n");
        }
    }
}