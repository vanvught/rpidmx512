EXTRA_INCLUDES+=../lib-lightset/include ../lib-properties/include ../lib-network/include

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_SRCDIR+=src/node
	endif
	
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(MAKE_FLAGS)), ARTNET_HAVE_DMXIN)
		EXTRA_SRCDIR+=src/node/dmxin
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
		EXTRA_SRCDIR+=src/node/rdm
		EXTRA_SRCDIR+=src/node/rdm/controller
		EXTRA_INCLUDES+=../lib-rdm/include ../lib-dmx/include
	endif
	
	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
		EXTRA_SRCDIR+=src/node/rdm
		EXTRA_SRCDIR+=src/node/rdm/responder
		EXTRA_INCLUDES+=../lib-rdm/include
	endif
	
	ifeq ($(findstring ARTNET_CONTROLLER,$(MAKE_FLAGS)), ARTNET_CONTROLLER)
		EXTRA_SRCDIR+=src/controller
	endif
	
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_SRCDIR+=src/node
	endif
	
	ifeq ($(findstring ARTNET_HAVE_FAILSAFE_RECORD,$(MAKE_FLAGS)), ARTNET_HAVE_FAILSAFE_RECORD)
		EXTRA_SRCDIR+=src/node/failsafe
		EXTRA_INCLUDES+=src/node/failsafe
	endif
			
	ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)), ARTNET_VERSION=4)
		EXTRA_SRCDIR+=src/node/4
		EXTRA_INCLUDES+=../lib-e131/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)), OUTPUT_DMX_SEND)
			EXTRA_INCLUDES+=../lib-dmx/include
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_ROM,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-flashcode/include
	endif
else
	EXTRA_SRCDIR+=src/node src/node/failsafe src/node/dmxin src/node/rdm src/node/rdm/controller src/node/timecode
	EXTRA_SRCDIR+=src/node/4
	EXTRA_INCLUDES+=src/node/failsafe
	EXTRA_INCLUDES+=../lib-e131/include
	EXTRA_INCLUDES+=../lib-dmx/include
	EXTRA_INCLUDES+=../lib-rdm/include
	DEFINES+=ARTNET_HAVE_TIMECODE
	DEFINES+=ARTNET_HAVE_FAILSAFE_RECORD
	DEFINES+=ARTNET_HAVE_DMXIN E131_HAVE_DMXIN
	DEFINES+=OUTPUT_HAVE_STYLESWITCH
	DEFINES+=OUTPUT_DMX_SEND
	DEFINES+=ARTNET_ENABLE_SENDDIAG
	DEFINES+=RDM_CONTROLLER
	DEFINES+=ARTNET_VERSION=4
	DEFINES+=LIGHTSET_PORTS=1
	DEFINES+=NODE_SHOWFILE 
	DEFINES+=CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET
endif