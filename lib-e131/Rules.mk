ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_SRCDIR+=src/node
	endif
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)),NODE_ARTNET)
		ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)),ARTNET_VERSION=4)
			EXTRA_SRCDIR+=src/node
		endif
	endif
	ifeq ($(findstring E131_HAVE_DMXIN,$(MAKE_FLAGS)), E131_HAVE_DMXIN)
		EXTRA_SRCDIR+=src/node/dmxin
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	ifeq ($(findstring E131_CONTROLLER,$(MAKE_FLAGS)), E131_CONTROLLER)
		EXTRA_SRCDIR+=src/controller
	endif
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_SRCDIR+=src/node
	endif
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)), OUTPUT_DMX_SEND)
			EXTRA_INCLUDES+=../lib-dmx/include
	endif
else
  EXTRA_SRCDIR+=src/node src/node/dmxin src/controller
  DEFINES+=E131_HAVE_DMXIN
  DEFINES+=OUTPUT_HAVE_STYLESWITCH
  DEFINES+=OUTPUT_DMX_SEND
  DEFINES+=LIGHTSET_PORTS=4
  EXTRA_INCLUDES+=../lib-dmx/include
  DEFINES+=NODE_SHOWFILE 
	DEFINES+=CONFIG_SHOWFILE_PROTOCOL_NODE_E131
endif