#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"

#define CMD_CNT 5
Command commands[] = {
    {"loc", cmd_loc, "Usage: loc (--address <address> | --alias <alias>) [OPTION]...\n", "Description: Gives access to the configuration for locomotives.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the locomotive which should be changed.\n  -A <alias>, --alias <alias>                                                  Select the alias of the locomotive which should be changed. Is internally resolved to the address which is configured for this alias.\n  -d (forward | backward), --direction (forward | backward)                    Set the direction in which the locomotive should drive.\n  -h, --help                                                                   Show this screen.\n  -l (on|off), --light (on|off)                                                Enable or disable the light of the locomotive.\n  --list                                                                       List the available locomotives.\n  -m, --monitor                                                                Shows the current configuration of the locomotive.\n  -s <stop | e-stop | 0-15>, --speed <stop | e-stop | 0-15>                    Set the speed the locomotive should drive.\n"},
    {"mag", cmd_mag, "Usage: mag (--address <address> | --alias <alias>) --device <device> [OPTION]...\n", "Description: Gives access to the configuration for magnetic accessories.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the accessory which should be changed.\n  -A <alias>, --alias <alias>                                                  Select the alias of the accessory which should be changed. Is internally resolved to the address which is configured for this alias.\n  -d <device>, --device <device>                                               Select the device (1-4) which which should be changed.\n  -s (on|off), --switch (on|off)                                               Enable or disable the switch.\n"},
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
                if (sscanf(value, "%d", &address) != 1)
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
            // Get the value for the direction
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
            // Get the value for the direction
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
            return;
        }
        else if (strcmp(option, "--list") == 0)
        {
            // TODO: List the available locomotives
            printf("Locomotives:\n");
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
        if (address < 0 && alias[0] != '\0')
        {
            // TODO: Resolve address from alias
        }

        if (monitor)
        {
            // TODO: Read the values from current state
            printf("%s (%s) - dir: %d, light: %d, speed: %d\n", address, alias, direction, light, speed);
            return;
        }

        printf("-------------\n");
        printf("Address: %d\n", address);
        printf("Alias: %s\n", alias[0] != '\0' ? alias : "Undefined");
        printf("Direction: %d\n", direction);
        printf("Light: %d\n", light);
        printf("Speed: %d\n", speed);
        printf("-------------\n");
        // TODO: Call corresponding functions
    }
    else
    {
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
    int a, b;
    if (sscanf(args, "%d %d", &a, &b) == 2) // TODO: add correct options scanning
    {
        printf("Result: %d\n", a + b);
        // TODO: Call corrsponding functions
    }
    else
    {
        for (int i = 0; i < CMD_CNT; i++)
        {
            if (strcmp("mag", commands[i].name) == 0)
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