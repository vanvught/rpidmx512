EXTRA_INCLUDES+=../lib-rdm/include ../lib-lightset/include ../lib-hal/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/artnet
	else
		EXTRA_INCLUDES+=
		EXTRA_SRCDIR+=src/rdm
	endif
else
	EXTRA_INCLUDES+=../lib-rdm/include ../lib-artnet/include ../lib-lightset/include 
	EXTRA_SRCDIR+=src/rdm src/artnet
endif