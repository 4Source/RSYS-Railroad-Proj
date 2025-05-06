#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

#define MAX_INPUT 1024

typedef struct
{
    const char *name;
    void (*func)(char *args);
    const char *usage;
    const char *description;
    const char *options;
} Command;

int prompt(char **command, char **args);
int handle_command(const char *command, char *args);
void cmd_loc(char *args);
void cmd_mag(char *args);
void cmd_restore(char *args);
void cmd_help(char *args);

#endif