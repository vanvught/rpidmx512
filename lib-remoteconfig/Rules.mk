EXTRA_INCLUDES =../lib-hal/include ../lib-network/include ../lib-properties/include

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_INCLUDES+=../lib-artnet/include
		ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)), ARTNET_VERSION=4)
			EXTRA_INCLUDES+=../lib-e131/include
		endif	
	endif
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_INCLUDES+=../lib-e131/include
	endif
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_INCLUDES+=../lib-node/include
		EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include
		EXTRA_INCLUDES+=../lib-rdmdiscovery/include
	endif
else
	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include
	EXTRA_INCLUDES+=../lib-rdmdiscovery/include
	EXTRA_INCLUDES+=../lib-node/include
endif

EXTRA_INCLUDES+=../lib-flash/include ../lib-flashcode/include
EXTRA_INCLUDES+=../lib-flashcodeinstall/include ../lib-configstore/include
EXTRA_INCLUDES+=../lib-device/include
EXTRA_INCLUDES+=../lib-displayudf/include ../lib-display/include
EXTRA_INCLUDES+=../lib-dmxmonitor/include ../lib-dmxreceiver/include ../lib-dmxsend/include ../lib-dmxserial/include ../lib-dmx/include
EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmresponder/include
EXTRA_INCLUDES+=../lib-ws28xxdmx/include ../lib-ws28xx/include ../lib-tlc59711dmx/include ../lib-tlc59711/include ../lib-ltc/include ../lib-tcnet/include ../lib-midi/include ../lib-oscserver/include ../lib-oscclient/include ../lib-widget/include ../lib-l6470dmx/include ../lib-l6470/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include ../lib-showfile/include ../lib-gps/include ../lib-rgbpanel/include ../lib-ddp/include ../lib-lightset/include
