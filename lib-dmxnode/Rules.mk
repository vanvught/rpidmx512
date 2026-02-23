EXTRA_INCLUDES+=
EXTRA_SRCDIR+=src/json
	
ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_INCLUDES+=../lib-artnet/include
	endif	
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_INCLUDES+=../lib-e131/include
	endif
	ifneq (,$(findstring RDM_RESPONDER,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdmsensor/include
	endif
	ifeq ($(findstring ARTNET_HAVE_FAILSAFE_RECORD,$(MAKE_FLAGS)), ARTNET_HAVE_FAILSAFE_RECORD)
		EXTRA_SRCDIR+=src/scenes
	endif
else
	DEFINES+=NODE_ARTNET
	DEFINES+=ARTNET_VERSION=4
	DEFINES+=DMXNODE_PORTS=4
	DEFINES+=OUTPUT_DMX_PIXEL
	DEFINES+=CONFIG_DMXNODE_PIXEL_MAX_PORTS=4
	EXTRA_SRCDIR+=src/json src/json/artnet src/json/e131
	EXTRA_INCLUDES+=../lib-rdmsensor/include
endif
