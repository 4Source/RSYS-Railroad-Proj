#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"

#define CMD_CNT 5
Command commands[] = {
    {"loc", cmd_loc, "Usage: loc (--address <address> | --alias <alias>) [OPTION]...\n", "Description: Gives access to the configuration for locomotives.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the locomotive which should be changed.\n  -A <alias>, --alias <alias>                                                  Select the alias of the locomotive which should be changed. Is internally resolved to the address which is configured for this alias.\n  -d (forward | backward), --direction (forward | backward)                    Set the direction in which the locomotive should drive.\n  -h, --help                                                                   Show this screen.\n  -l (on|off), --light-on (on|off)                                             Enable or disable the light of the locomotive.\n  --list                                                                       List the available locomotives.\n  -m, --monitor                                                                Shows the current configuration of the locomotive.\n  -s <Stop | E-Stop | Step1..28>, --speed <Stop | E-Stop | Step1..28>          Set the speed the locomotive should drive.\n"},
    {"mag", cmd_mag, "Usage: mag (--address <address> | --alias <alias>) --device <device> [OPTION]...\n", "Description: Gives access to the configuration for magnetic accessories.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the accessory which should be changed.\n  -A <alias>, --alias <alias>                                                  Select the alias of the accessory which should be changed. Is internally resolved to the address which is configured for this alias.\n  -d <device>, --device <device>                                               Select the device (1-4) which which should be changed.\n  -s (on|off), --switch (on|off)                                               Enable or disable the switch.\n"},
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
            if (strcmp("loc", commands[i].name) == 0)
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