$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_SRCDIR+=src/artnet
	endif
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_SRCDIR+=src/e131
	endif
	ifeq ($(findstring NODE_LTC_SMPTE,$(MAKE_FLAGS)), NODE_LTC_SMPTE)
		EXTRA_SRCDIR+=src/ltc
	endif
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_SRCDIR+=src/node
	endif
	ifeq ($(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)), OUTPUT_DMX_PIXEL)
		EXTRA_SRCDIR+=src/pixel
	endif
	ifeq ($(findstring OUTPUT_DMX_STEPPER,$(MAKE_FLAGS)), OUTPUT_DMX_STEPPER)
		EXTRA_SRCDIR+=src/stepper
	endif
else
	EXTRA_SRCDIR+=src/artnet src/ddp src/e131 src/ltc src/node src/stepper
	DEFINES+=LIGHTSET_PORTS=4
	DEFINES+=CONFIG_PIXELDMX_MAX_PORTS=8
	DEFINES+=CONFIG_DDPDISPLAY_MAX_PORTS=8
endif

EXTRA_INCLUDES =../lib-hal/include ../lib-network/include ../lib-properties/include ../lib-lightset/include

EXTRA_INCLUDES+=../lib-remoteconfig/include
EXTRA_INCLUDES+=../lib-spiflash/include ../lib-spiflashinstall/include ../lib-spiflashstore/include
EXTRA_INCLUDES+=../lib-device/include
EXTRA_INCLUDES+=../lib-displayudf/include ../lib-display/include
EXTRA_INCLUDES+=../lib-dmxmonitor/include ../lib-dmxreceiver/include ../lib-dmxsend/include ../lib-dmxserial/include ../lib-dmx/include
EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmresponder/include
EXTRA_INCLUDES+=../lib-e131/include
EXTRA_INCLUDES+=../lib-artnet/include ../lib-artnet4/include ../lib-rdmdiscovery/include ../lib-rdm/include
EXTRA_INCLUDES+=../lib-ws28xxdmx/include ../lib-ws28xx/include ../lib-tlc59711dmx/include ../lib-tlc59711/include 
EXTRA_INCLUDES+=../lib-ltc/include ../lib-tcnet/include ../lib-midi/include ../lib-oscserver/include ../lib-oscclient/include ../lib-widget/include ../lib-l6470dmx/include ../lib-l6470/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include ../lib-showfile/include ../lib-gps/include ../lib-rgbpanel/include ../lib-ddp/include
EXTRA_INCLUDES+=../lib-node/include