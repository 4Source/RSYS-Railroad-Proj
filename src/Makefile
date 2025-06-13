CC := gcc

# Get the path of the current directory and store it
PWD	:= /home/ezslabor/RSYS-Railroad-Proj
# The path to the scr directory
SRC_DIR := $(PWD)/src
# The path to the build directory
BUILD_DIR := $(PWD)/build
# The path to the include directory
INCLUDE_DIR := $(PWD)/include

# Set the name of the kernel module
obj-m	:= rtai_main.o
# rtai_main-y += telegram/locomotive.o telegram/magnetic.o telegram/idle.o telegram/reset.o
# rtai_main-y += communication/railroad_communication.o 
# rtai_main-y += communication/rtai_linux_communication.o

# Flags to give to the compiler
ccflags-y := -I/usr/realtime/include -I/usr/src/linux/include 
ccflags-y += -I$(INCLUDE_DIR)

# Get the kernel modules path for current version
KDIR	:= /lib/modules/$(shell uname -r)/build

# Make rtai kernel module
rtai_module:
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) modules

# Make user interface program
interface_main: main.c command.c telegram/locomotive.c telegram/magnetic.c communication/linux_rtai_communication.c
	gcc main.c command.c telegram/locomotive.c telegram/magnetic.c communication/linux_rtai_communication.c -I/usr/realtime/include -I $(INCLUDE_DIR) -o dcc
	chmod u+x dcc

clean:
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) clean


