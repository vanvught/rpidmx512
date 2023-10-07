$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES =../lib-flashcode/include ../lib-flash/include
EXTRA_INCLUDES+=../lib-hal/include ../lib-properties/include ../lib-lightset/include ../lib-network/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONFIG_STORE_USE_FILE,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/file
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_I2C,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/i2c
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_RAM,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/ram
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_ROM,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/rom
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_SPI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/spi
	endif
	
	RDM=
	
	ifneq (,$(findstring ESP8266,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/network
		EXTRA_INCLUDES+=../lib-network/include
		# Remote config is not used with ESP8266
		EXTRA_INCLUDES+=../lib-remoteconfig/include
	endif
	
	ifeq (,$(findstring NO_EMAC,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/network
		EXTRA_INCLUDES+=../lib-network/include
		EXTRA_INCLUDES+=../lib-remoteconfig/include
	endif
	
	ifeq ($(findstring DISPLAY_UDF,$(MAKE_FLAGS)), DISPLAY_UDF)
		EXTRA_SRCDIR+=src/displayudf
		EXTRA_INCLUDES+=../lib-displayudf/include
	endif

	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_SRCDIR+=src/artnet
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/rdm
		RDM=1
		EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include
		ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)), ARTNET_VERSION=4)
			EXTRA_INCLUDES+=../lib-e131/include
		endif	
	endif
	
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_SRCDIR+=src/e131
		EXTRA_INCLUDES+=../lib-e131/include
	endif
	
	ifeq ($(findstring NODE_LTC_SMPTE,$(MAKE_FLAGS)), NODE_LTC_SMPTE)
		EXTRA_SRCDIR+=src/ltc
		EXTRA_INCLUDES+=../lib-ltc/include ../lib-tcnet/include
		EXTRA_INCLUDES+=../lib-gps/include
		EXTRA_INCLUDES+=../lib-rgbpanel/include
		EXTRA_INCLUDES+=../lib-ws28xx/include
	endif
	
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_SRCDIR+=src/node
		EXTRA_INCLUDES+=../lib-node/include
		EXTRA_INCLUDES+=../lib-artnet/include ../lib-rdmdiscovery/include
		EXTRA_INCLUDES+=../lib-e131/include
	endif
	
	ifeq ($(findstring NODE_OSC_CLIENT,$(MAKE_FLAGS)), NODE_OSC_CLIENT)
		EXTRA_SRCDIR+=src/oscclient
		EXTRA_INCLUDES+=../lib-oscclient/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)),OUTPUT_DMX_SEND)
		EXTRA_SRCDIR+=src/dmx
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)), OUTPUT_DMX_PIXEL)
		EXTRA_SRCDIR+=src/pixel
		EXTRA_INCLUDES+=../lib-ws28xxdmx/include ../lib-ws28xx/include
	endif

	ifeq ($(findstring OUTPUT_DMX_SHOWFILE,$(MAKE_FLAGS)), OUTPUT_DMX_SHOWFILE)
		EXTRA_SRCDIR+=src/showfile
		EXTRA_INCLUDES+=../lib-showfile/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_SERIAL,$(MAKE_FLAGS)), OUTPUT_DMX_SERIAL)
		EXTRA_SRCDIR+=src/dmxserial
		EXTRA_INCLUDES+=../lib-dmxserial/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_STEPPER,$(MAKE_FLAGS)), OUTPUT_DMX_STEPPER)
		EXTRA_SRCDIR+=src/stepper
		EXTRA_INCLUDES+=../lib-l6470dmx/include ../lib-l6470/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_TLC59711,$(MAKE_FLAGS)), OUTPUT_DMX_TLC59711)
		EXTRA_SRCDIR+=src/tlc59711
		EXTRA_INCLUDES+=../lib-tlc59711dmx/include ../lib-tlc59711/include
	endif
	
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
		ifdef RDM
		else
			EXTRA_SRCDIR+=src/rdm
			RDM=1
		endif
	endif
	
	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
		ifdef RDM
		else
			EXTRA_SRCDIR+=src/rdm
			RDM=1
		endif
		EXTRA_INCLUDES+=../lib-rdmresponder/include
	endif
	
	ifeq ($(findstring NODE_RDMNET_LLRP_ONLY,$(MAKE_FLAGS)), NODE_RDMNET_LLRP_ONLY)
		ifdef RDM
		else
			EXTRA_SRCDIR+=src/rdm
			RDM=1
		endif
		EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include		
	endif
	
	ifeq ($(findstring WIDGET_HAVE_FLASHROM,$(MAKE_FLAGS)), WIDGET_HAVE_FLASHROM)
		EXTRA_SRCDIR+=src/widget
		EXTRA_INCLUDES+=../lib-widget/include
	endif
else
	EXTRA_SRCDIR+=src/artnet
	EXTRA_INCLUDES+=../lib-artnet/include
	EXTRA_SRCDIR+=src/e131
	EXTRA_INCLUDES+=../lib-e131/include
	EXTRA_SRCDIR+=src/node 
	EXTRA_INCLUDES+=../lib-node/include ../lib-rdmdiscovery/include
	EXTRA_SRCDIR+=src/ltc
	EXTRA_INCLUDES+=../lib-ltc/include ../lib-tcnet/include
	EXTRA_INCLUDES+=../lib-gps/include
	EXTRA_INCLUDES+=../lib-rgbpanel/include
	EXTRA_INCLUDES+=../lib-ws28xx/include
	EXTRA_SRCDIR+=src/rdm
	EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include		
	EXTRA_SRCDIR+=src/stepper
	EXTRA_INCLUDES+=../lib-l6470dmx/include ../lib-l6470/include
	EXTRA_INCLUDES+=../lib-tlc59711dmx/include ../lib-tlc59711/include
	
	DEFINES+=ARTNET_VERSION=4
	DEFINES+=LIGHTSET_PORTS=4
	DEFINES+=CONFIG_PIXELDMX_MAX_PORTS=8
	DEFINES+=CONFIG_DDPDISPLAY_MAX_PORTS=8
endif

EXTRA_INCLUDES+=../lib-displayudf/include ../lib-display/include
EXTRA_INCLUDES+=../lib-dmxsend/include
EXTRA_INCLUDES+=../lib-dmxmonitor/include
EXTRA_INCLUDES+=../lib-dmxreceiver/include ../lib-dmx/include
EXTRA_INCLUDES+=../lib-oscserver/include 
EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include ../lib-rdmsubdevice/include
EXTRA_INCLUDES+=../lib-spiflashinstall/include
EXTRA_INCLUDES+=../lib-device/include
EXTRA_INCLUDES+=../lib-midi/include