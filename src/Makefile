CC := gcc

# Set the name of the kernel module
obj-m	:= rtai_main.o
rtai_main-y := rtai_main.o
rtai_main-y += communication/rtai_linux_communication.o
rtai_main-y += communication/railroad_communication.o
rtai_main-y += communication/railroad_communication.o
rtai_main-y += telegram/locomotive.o
rtai_main-y += telegram/magnetic.o
# Flags to give to the compiler
ccflags-y := -I/usr/realtime/include -I/usr/src/linux/include 
ccflags-y += -I$(INCLUDE_DIR)

# Get the path of the current directory and store it
PWD	:= $(shell pwd)/..
# The path to the scr directory
SRC_DIR := $(PWD)/src
# The path to the build directory
BUILD_DIR := $(PWD)/build
# The path to the include directory
INCLUDE_DIR := $(PWD)/include

# Set the name of the kernel module
obj-m	:= rtai_main.o
rtai_main-y := $(SRC_DIR)/rtai_main.o
rtai_main-y += $(SRC_DIR)/communication/rtai_linux_communication.o
rtai_main-y += $(SRC_DIR)/communication/railroad_communication.o
rtai_main-y += $(SRC_DIR)/communication/railroad_communication.o
rtai_main-y += $(SRC_DIR)/telegram/locomotive.o
rtai_main-y += $(SRC_DIR)/telegram/magnetic.o
# Flags to give to the compiler
ccflags-y := -I/usr/realtime/include -I/usr/src/linux/include 
ccflags-y += -I/home/ezslabor/RSYS-Railroad-Proj/include

# Get the kernel modules path for current version
KDIR	:= /lib/modules/$(shell uname -r)/build

# Make rtai kernel module
rtai_main: rtai_main.c
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) modules

# Make user interface program
interface_main: main.c
	gcc main.c -I $(INCLUDE_DIR) -o dcc
	chmod u+x dcc

clean:
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) clean


