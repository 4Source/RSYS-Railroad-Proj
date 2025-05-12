#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

obj-m := logic.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
SRC_DIR := $(PWD)/src
BUILD_DIR := $(PWD)/build
EXTRA_CFLAGS := -I/usr/realtime/include -I/usr/src/linux/include -I$(SRC_DIR)/include

default:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(SRC_DIR) O=$(BUILD_DIR) modules

clean:
	rm -rf $(BUILD_DIR)
