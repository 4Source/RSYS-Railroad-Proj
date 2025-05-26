build_all: rtai_main interface_main

rtai_main:
	$(MAKE) -C src rtai_main

interface_main:
	$(MAKE) -C src interface_main

clean:
	$(MAKE) -C src clean