# Get the path of the current directory and store it
PWD	:= $(shell pwd)
# The path to the scr directory
SRC_DIR := $(PWD)/src
# The path to the build directory
BUILD_DIR := $(PWD)/build
# The path to the include directory
INCLUDE_DIR := $(PWD)/include

build_all: rtai_main interface_main

rtai_main:
	$(MAKE) -C src rtai_main PWD=$(PWD) SRC_DIR=$(SRC_DIR) BUILD_DIR=$(BUILD_DIR) INCLUDE_DIR=$(INCLUDE_DIR)

interface_main:
	$(MAKE) -C src interface_main PWD=$(PWD) SRC_DIR=$(SRC_DIR) BUILD_DIR=$(BUILD_DIR) INCLUDE_DIR=$(INCLUDE_DIR)

clean:
	$(MAKE) -C src clean