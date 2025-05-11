#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

obj-m	:= logic.o

PWD	:= $(shell pwd)
KDIR	:= $(PWD)/build
SRCDIR	:= $(PWD)/src
EXTRA_CFLAGS := -I /usr/realtime/include -I/usr/src/linux/include -I$(PWD)/include
EXTRA_LFLAGS := -L/usr/realtime/lib -lpthread

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(SRCDIR) SRC=$(SRCDIR)/logic.c modules

clean:
	rm -r .tmp_versions
	rm  .`basename $(obj-m) .o`.*
	rm `basename $(obj-m) .o`.o
	rm `basename $(obj-m) .o`.ko
	rm `basename $(obj-m) .o`.mod.*
