$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES=../lib-network/include ../lib-properties/include ../lib-display/include ../lib-lightset/include
EXTRA_INCLUDES+=../lib-flashcode/include ../lib-flashcodeinstall/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring ENABLE_HTTPD,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/httpd
	endif
	
	ifneq (,$(findstring ENABLE_SHELL,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/shell src/uart
	endif
	
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
		EXTRA_INCLUDES+=../lib-rdmdiscovery/include ../lib-rdm/include
	endif
	ifeq ($(findstring NODE_LTC_SMPTE,$(MAKE_FLAGS)), NODE_LTC_SMPTE)
		EXTRA_INCLUDES+=../lib-ltc/include ../lib-tcnet/include ../lib-gps/include ../lib-midi/include 
		EXTRA_INCLUDES+=../lib-rgbpanel/include ../lib-ws28xx/include
	endif
	ifeq ($(findstring NODE_OSC_CLIENT,$(MAKE_FLAGS)), NODE_OSC_CLIENT)
		EXTRA_INCLUDES+=../lib-osc/include
	endif
	ifeq ($(findstring NODE_OSC_SERVER,$(MAKE_FLAGS)), NODE_OSC_SERVER)
		EXTRA_INCLUDES+=../lib-osc/include
	endif
	ifeq ($(findstring NODE_SHOWFILE,$(MAKE_FLAGS)), NODE_SHOWFILE)
		EXTRA_INCLUDES+=../lib-showfile/include
		ifeq ($(findstring CONFIG_SHOWFILE_ENABLE_OSC,$(MAKE_FLAGS)), CONFIG_SHOWFILE_ENABLE_OSC)
			EXTRA_INCLUDES+=../lib-osc/include
		endif
		ifeq ($(findstring CONFIG_SHOWFILE_PROTOCOL_E131,$(MAKE_FLAGS)), CONFIG_SHOWFILE_PROTOCOL_E131)
			EXTRA_INCLUDES+=../lib-e131/include
		endif
		ifeq ($(findstring CONFIG_SHOWFILE_PROTOCOL_ARTNET,$(MAKE_FLAGS)), CONFIG_SHOWFILE_PROTOCOL_ARTNET)
			EXTRA_INCLUDES+=../lib-artnet/include
		endif
	endif
	
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
		EXTRA_INCLUDES+=../lib-rdm/include ../lib-dmx/include
	endif
	
	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
		EXTRA_INCLUDES+=../lib-rdmresponder/include ../lib-rdm/include
		EXTRA_INCLUDES+=../lib-rdmsensor/include ../lib-rdmsubdevice/include
		EXTRA_INCLUDES+=../lib-dmxreceiver/include ../lib-dmx/include
	endif
	
	ifeq ($(findstring DISPLAY_UDF,$(MAKE_FLAGS)), DISPLAY_UDF)
		EXTRA_INCLUDES+=../lib-displayudf/include
	endif
	
	ifeq ($(findstring WIDGET_HAVE_FLASHROM,$(MAKE_FLAGS)), WIDGET_HAVE_FLASHROM)
		EXTRA_INCLUDES+=../lib-widget/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)),OUTPUT_DMX_SEND)
		EXTRA_INCLUDES+=../lib-dmxsend/include ../lib-dmx/include
	endif
	ifeq ($(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)), OUTPUT_DMX_PIXEL)
		EXTRA_INCLUDES+=../lib-ws28xxdmx/include ../lib-ws28xx/include
	endif
	ifeq ($(findstring OUTPUT_DMX_MONITOR,$(MAKE_FLAGS)), OUTPUT_DMX_MONITOR)
		EXTRA_INCLUDES+=../lib-dmxmonitor/include
	endif
	ifeq ($(findstring OUTPUT_DMX_SERIAL,$(MAKE_FLAGS)), OUTPUT_DMX_SERIAL)
		EXTRA_INCLUDES+=../lib-dmxserial/include
	endif
	ifeq ($(findstring OUTPUT_DMX_STEPPER,$(MAKE_FLAGS)), OUTPUT_DMX_STEPPER)
		EXTRA_INCLUDES+=../lib-l6470dmx/include ../lib-l6470/include
		EXTRA_INCLUDES+=../lib-tlc59711dmx/include ../lib-tlc59711/include
	endif
	ifeq ($(findstring OUTPUT_DMX_SHOWFILE,$(MAKE_FLAGS)), OUTPUT_DMX_SHOWFILE)
		EXTRA_INCLUDES+=../lib-showfile/include
	endif
	ifeq ($(findstring OUTPUT_DMX_PCA9685,$(MAKE_FLAGS)), OUTPUT_DMX_PCA9685)
		EXTRA_INCLUDES+=../lib-pca9685dmx/include ../lib-pca9685/include
	endif
else
	EXTRA_SRCDIR+=src/httpd
	DEFINES+=ENABLE_CONTENT

	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include
	EXTRA_INCLUDES+=../lib-rdmdiscovery/include
	EXTRA_INCLUDES+=../lib-node/include
	EXTRA_INCLUDES+=../lib-ltc/include ../lib-tcnet/include
	EXTRA_INCLUDES+=../lib-gps/include
	EXTRA_INCLUDES+=../lib-rgbpanel/include
	EXTRA_INCLUDES+=../lib-displayudf/include
	EXTRA_INCLUDES+=../lib-widget/include
	EXTRA_INCLUDES+=../lib-dmxsend/include ../lib-dmx/include ../lib-dmxreceiver/include
	EXTRA_INCLUDES+=../lib-ws28xxdmx/include ../lib-ws28xx/include
	EXTRA_INCLUDES+=../lib-tlc59711dmx/include ../lib-tlc59711/include
	EXTRA_INCLUDES+=../lib-dmxserial/include
	EXTRA_INCLUDES+=../lib-pca9685dmx/include ../lib-pca9685/include
	EXTRA_INCLUDES+=../lib-midi/include
	EXTRA_INCLUDES+=../lib-rdmresponder/include ../lib-rdm/include
	EXTRA_INCLUDES+=../lib-rdmsensor/include ../lib-rdmsubdevice/include
	EXTRA_INCLUDES+=../lib-showfile/include
	EXTRA_INCLUDES+=../lib-dmxmonitor/include 
	EXTRA_INCLUDES+=../lib-osc/include 
	
	DEFINES+=ARTNET_VERSION=4
	DEFINES+=RDM_CONTROLLER ENABLE_NET_PHYSTATUS CONFIG_USB_HOST_MSC ENABLE_PHY_SWITCH
	DEFINES+=NODE_SHOWFILE CONFIG_SHOWFILE_FORMAT_OLA CONFIG_SHOWFILE_PROTOCOL_E131
endif
