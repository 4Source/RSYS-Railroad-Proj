CC := gcc
# Set the name of the kernel module
obj-m	:= railroad_controller.o
railroad_controller-y := logic.o
# Flags to give to the compiler
ccflags-y := -I/usr/realtime/include -I/usr/src/linux/include -I$(shell pwd)/include


# Get the kernel modules path for current version
KDIR	:= /lib/modules/$(shell uname -r)/build
# Get the path of the current directory and store it
PWD	:= $(shell pwd)/..
# The path to the scr directory
SRC_DIR := $(PWD)/src
# The path to the build directory
BUILD_DIR := $(PWD)/build
# The path to the include directory
INCLUDE_DIR := $(PWD)/include

# Make rtai kernel module
rtai_main: logic.c
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) modules

# Make user interface program
interface_main: main.c
	gcc main.c -I $(INCLUDE_DIR) -o dcc
	chmod u+x dcc

clean:
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) clean


