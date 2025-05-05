#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "magnetic.h"
#include "locomotive.h"

#define MAX_INPUT 1024
#define CMD_CNT 5

void cmd_loc(char *args);
void cmd_mag(char *args);
void cmd_help(char *args);

typedef struct
{
    const char *name;
    void (*func)(char *args);
    const char *usage;
    const char *description;
    const char *options;
} Command;

Command commands[] = {
    {"loc", cmd_loc, "Usage: loc (--address <address> | --alias <alias>) [OPTION]...\n", "Description: Gives access to the configuration for locomotives.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the locomotive which should be changed.\n  -A <alias>, --alias <alias>                                                  Select the alias of the locomotive which should be changed. Is internally resolved to the address which is configured for this alias.\n  -d (forward | backward), --direction (forward | backward)                    Set the direction in which the locomotive should drive.\n  -h, --help                                                                   Show this screen.\n  -l (on|off), --light-on (on|off)                                             Enable or disable the light of the locomotive.\n  --list                                                                       List the available locomotives.\n  -m, --monitor                                                                Shows the current configuration of the locomotive.\n  -s <Stop | E-Stop | Step1..28>, --speed <Stop | E-Stop | Step1..28>          Set the speed the locomotive should drive.\n"},
    {"mag", cmd_mag, "Usage: mag (--address <address> | --alias <alias>) --device <device> [OPTION]...\n", "Description: Gives access to the configuration for magnetic accessories.\n", "Options:\n  -a <address>, --address <address>                                            Select the address of the accessory which should be changed.\n  -A <alias>, --alias <alias>                                                  Select the alias of the accessory which should be changed. Is internally resolved to the address which is configured for this alias.\n  -d <device>, --device <device>                                               Select the device (1-4) which which should be changed.\n  -s (on|off), --switch (on|off)                                               Enable or disable the switch.\n"},
    {"help", cmd_help, "Usage: help\n       help <command>\n", "Description: Show this help or used with <command> --help.\n", ""},
    {"exit", NULL, "Usage: exit\n", "Description: Terminates the current prompt. Also sends a reset message to all decoders.\n", ""},
    {NULL, NULL, NULL}};

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

int main()
{
    char input[MAX_INPUT];

    while (1)
    {
        printf("dcc> ");
        fflush(stdout);

        if (!fgets(input, MAX_INPUT, stdin))
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        char *command = strtok(input, " ");
        char *args = strtok(NULL, "");

        if (!command)
        {
            continue;
        }

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
    }

    return 0;
}
