# RSYS-Railroad-Proj

# Help
Start the program with ``dcc`` this loads modules in RTAI and opens the prompt. 

## Commands
```
Usage: loc (--address <address> | --alias <alias>) [OPTION]...

Description: Gives access to the configuration for locomotives.

Options:
  -a <address>, --address <address>                                            Select the address of the locomotive which should be changed. Address range is 0 to 127.
  -A <alias>, --alias <alias>                                                  Select the alias of the locomotive which should be changed. Is internally resolved to the address which is configured for this alias. 
  -d (forward | backward), --direction (forward | backward)                    Set the direction in which the locomotive should drive.
  -h, --help                                                                   Show this screen.
  -l (on|off), --light (on|off)                                                Enable or disable the light of the locomotive.
  --list                                                                       List the available locomotives.
  -m, --monitor                                                                Shows the current configuration of the locomotive.
  -s <stop | e-stop | 0-15>, --speed <stop | e-stop | 0-15>                    Set the speed the locomotive should drive. 
```
```
Usage: mag (--address <address> | --alias <alias>) --device <device> [OPTION]...

Description: Gives access to the configuration for magnetic accessories.

Options:
  -a <address>, --address <address>                                            Select the address of the accessory which should be changed. Address range is 0 to 511.
  -A <alias>, --alias <alias>                                                  Select the alias of the accessory which should be changed. Is internally resolved to the address which is configured for this alias.
  -d <device>, --device <device>                                               Select the device (1-4) which which should be changed.
  -s (on|off), --switch (on|off)                                               Enable or disable the switch.
```
```
Usage: restore [digital]

Description: Restores the digital mode. Use this command if the system has switched to an alternative mode (analog mode).
```
```
Usage: exit

Description: Terminates the current prompt. Also sends a reset message to all decoders.
```
```
Usage: help
       help <command>

Description: Show this help or used with <command> --help.
```