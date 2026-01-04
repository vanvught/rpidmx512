EXTRA_SRCDIR+=src/json

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/artnet	
		ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)), ARTNET_VERSION=4)
			EXTRA_INCLUDES+=../lib-e131/include
		endif	
	endif
	
	ifneq (,$(findstring NODE_E131,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-e131/include
		EXTRA_SRCDIR+=src/e131
	endif
	
	ifneq (,$(findstring NODE_DDP_DISPLAY,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-ddp/include
	endif
	
	ifeq (,$(findstring NO_EMAC,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-network/include
		EXTRA_SRCDIR+=src/network
	endif
	
	ifneq (,$(findstring RDM_CONTROLLER,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdm/include
	endif
	
	ifneq (,$(findstring RDM_RESPONDER,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdmresponder/include ../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include
		EXTRA_INCLUDES+=../lib-pixeldmx/include
		ifneq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
		else
			EXTRA_INCLUDES+=../lib-dmxreceiver/include ../lib-dmx/include
		endif
	endif
	
	ifneq (,$(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-dmxled/include
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_ROM,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-flashcode/include
	endif
else
	ifneq (, $(shell test -d '../lib-network/src/noemac' && echo -n yes))
		DEFINES+=NO_EMAC
	else
		DEFINES+=NODE_ARTNET NODE_E131 OUTPUT_DMX_ARTNET
		DEFINES+=ARTNET_VERSION=4
		EXTRA_SRCDIR+=src/artnet src/e131 src/network
		EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include ../lib-network/include
		EXTRA_INCLUDES+=../lib-node/include
	endif
	
	DEFINES+=RDM_RESPONDER
	DEFINES+=DMXNODE_PORTS=4
	
	EXTRA_INCLUDES+=../lib-dmxreceiver/include ../lib-dmx/include
	EXTRA_INCLUDES+=../lib-rdmdiscovery/include
	EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include 
endif