CC := gcc
# Set the name of the kernel module
obj-m	:= logic.o
logic-y := src/telegram/idle.o src/telegram/locomotive.o src/telegram/magnetic.o src/telegram/reset.o

# Get the kernel modules path for current version
KDIR	:= /lib/modules/$(shell uname -r)/build
# Get the path of the current directory and store it
PWD	:= $(shell pwd)
# The path to the scr directory
SRC_DIR := $(PWD)/src
# The path to the build directory
BUILD_DIR := $(PWD)/build
# Flags to give to the compiler
CFLAGS := -I/usr/realtime/include -I/usr/src/linux/include -I$(PWD)/include
# Flags to give to the linker
#LDFLAGS := 

# Make all 
all: rtai # command

# Make rtai kernel module
rtai: logic.c
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) MO=$(BUILD_DIR) modules

# Make user interface program
# command: command.c
# 	gcc -c command.c

clean:
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) MO=$(BUILD_DIR) clean


