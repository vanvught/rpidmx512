EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include ../lib-lightset/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/artnet
	else
		EXTRA_INCLUDES+=../lib-dmx/include ../lib-dmxreceiver/include
		EXTRA_SRCDIR+=src/rdm
	endif
else
	EXTRA_INCLUDES+=../lib-dmx/include ../lib-dmxreceiver/include ../lib-artnet/include
	EXTRA_SRCDIR+=src/rdm src/artnet
endif