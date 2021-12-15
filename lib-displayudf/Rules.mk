ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/artnet	
	endif
	ifneq (,$(findstring NODE_E131,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-e131/include
		EXTRA_SRCDIR+=src/e131
	endif
	ifeq (,$(findstring NO_EMAC,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-network/include
		EXTRA_SRCDIR+=src/network
	endif
	ifneq (,$(findstring RDM_RESPONDER,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdmresponder/include ../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include
		ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		else
			EXTRA_INCLUDES+=../lib-dmxreceiver/include ../lib-dmx/include
		endif
	endif
else
	DEFINES+=NODE_ARTNET NODE_E131 OUTPUT_DMX_ARTNET RDM_RESPONDER
	
	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include ../lib-network/include
	EXTRA_INCLUDES+=../lib-dmxreceiver/include ../lib-dmx/include
	EXTRA_INCLUDES+=../lib-rdmresponder/include ../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include 
	
	EXTRA_SRCDIR+=src/artnet src/e131 src/network
endif