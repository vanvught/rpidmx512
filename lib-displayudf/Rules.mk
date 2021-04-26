ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		EXTRA_INCLUDES += ../lib-artnet/include
		EXTRA_SRCDIR += src/artnet	
	endif
		ifneq (,$(findstring NODE_E131,$(MAKE_FLAGS)))
		EXTRA_INCLUDES += ../lib-e131/include
		EXTRA_SRCDIR += src/e131
	endif
else
	DEFINES += NODE_ARTNET NODE_E131
	EXTRA_INCLUDES += ../lib-artnet/include ../lib-e131/include
	EXTRA_SRCDIR += src/artnet src/e131
endif