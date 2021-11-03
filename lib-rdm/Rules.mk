EXTRA_INCLUDES=../lib-rdmsensor/include ../lib-rdmsubdevice/include ../lib-dmx/include ../lib-properties/include ../lib-lightset/include
EXTRA_INCLUDES+=../lib-hal/include ../lib-network/include ../lib-display/include 

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_RDMNET_LLRP_ONLY,$(MAKE_FLAGS)))
		EXTRA_SRCDIR += src/llrp
	endif
else
	EXTRA_SRCDIR += src/llrp
endif