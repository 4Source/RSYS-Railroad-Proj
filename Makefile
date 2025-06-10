build_all: rtai_main interface_main

rtai_module:
	$(MAKE) -C src rtai_module

interface_main:
	$(MAKE) -C src interface_main

clean:
	$(MAKE) -C src clean