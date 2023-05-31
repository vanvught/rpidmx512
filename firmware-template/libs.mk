$(info $$DEFINES [${DEFINES}])

ifeq ($(findstring NO_EMAC,$(DEFINES)),NO_EMAC)
else
	LIBS+=remoteconfig
endif

ifeq ($(findstring NODE_NODE,$(DEFINES)),NODE_NODE)
	LIBS+=node artnet e131
	DEFINES+=ARTNET_HAVE_FAILSAFE_RECORD
	ARTNET=1
endif

ifeq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
	ifeq ($(findstring ARTNET_VERSION=3,$(DEFINES)),ARTNET_VERSION=3)
		LIBS+=artnet
	else
		LIBS+=artnet4 artnet e131 
		ifeq ($(findstring ARTNET_HAVE_DMXIN,$(DEFINES)),ARTNET_HAVE_DMXIN)
			LIBS+=dmxartnet
		endif
	endif
endif

ifeq ($(findstring NODE_E131,$(DEFINES)),NODE_E131)
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131
	endif
	ifeq ($(findstring E131_HAVE_DMXIN,$(DEFINES)),E131_HAVE_DMXIN)
		LIBS+=dmxe131
	endif
endif

ifeq ($(findstring NODE_SHOWFILE,$(DEFINES)),NODE_SHOWFILE)
	LIBS+=showfile osc
endif

ifeq ($(findstring NODE_LTC_SMPTE,$(DEFINES)),NODE_LTC_SMPTE)
	LIBS+=ltc tcnet midi input osc ws28xxdisplay ws28xx rgbpanel gps
endif

ifeq ($(findstring NODE_OSC_CLIENT,$(DEFINES)),NODE_OSC_CLIENT)
	LIBS+=oscclient osc
endif

ifeq ($(findstring NODE_OSC_SERVER,$(DEFINES)),NODE_OSC_SERVER)
	LIBS+=oscserver osc
endif

ifeq ($(findstring NODE_DDP_DISPLAY,$(DEFINES)),NODE_DDP_DISPLAY)
	LIBS+=ddp
endif

ifeq ($(findstring NODE_PP,$(DEFINES)),NODE_PP)
	LIBS+=pp
endif

ifeq ($(findstring ARTNET_CONTROLLER,$(DEFINES)),ARTNET_CONTROLLER)
	ifneq ($(findstring artnet,$(LIBS)),artnet)
		LIBS+=artnet
	endif
endif

ifeq ($(findstring RDM_CONTROLLER,$(DEFINES)),RDM_CONTROLLER)
	LIBS+=rdmdiscovery rdm
	DMX=1
endif


ifeq ($(findstring RDM_RESPONDER,$(DEFINES)),RDM_RESPONDER)
	ifneq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
		ifneq ($(findstring dmxreceiver,$(LIBS)),dmxreceiver)
			LIBS+=dmxreceiver
		endif
	endif
	ifneq ($(findstring rdmresponder,$(LIBS)),rdmresponder)
		LIBS+=rdmresponder
	endif
	ifneq ($(findstring rdmsensor,$(LIBS)),rdmsensor)
		LIBS+=rdmsensor
	endif
	ifneq ($(findstring rdmsubdevice,$(LIBS)),rdmsubdevice)
		LIBS+=rdmsubdevice
	endif
	LIBS+=rdm
	DMX=1
endif

ifeq ($(findstring NODE_DMX,$(DEFINES)),NODE_DMX)
	LIBS+=dmxreceiver
	DMX=1
endif

ifeq ($(findstring NODE_RDMNET_LLRP_ONLY,$(DEFINES)),NODE_RDMNET_LLRP_ONLY)
	ifneq ($(findstring rdmnet,$(LIBS)),rdmnet)
		LIBS+=rdmnet
	endif
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131
	endif
	ifneq ($(findstring rdmsensor,$(LIBS)),rdmsensor)
		LIBS+=rdmsensor
	endif
	ifneq ($(findstring rdmsubdevice,$(LIBS)),rdmsubdevice)
		LIBS+=rdmsubdevice
	endif
	ifneq ($(findstring RDM_CONTROLLER,$(DEFINES)),RDM_CONTROLLER)
		LIBS+=rdm
	endif
endif

ifeq ($(findstring e131,$(LIBS)),e131)
	LIBS+=uuid
endif

ifeq ($(findstring OUTPUT_DMX_MONITOR,$(DEFINES)),OUTPUT_DMX_MONITOR)
	LIBS+=dmxmonitor	
endif

ifeq ($(findstring OUTPUT_DMX_SEND,$(DEFINES)),OUTPUT_DMX_SEND)
	LIBS+=dmxsend
	DMX=1
endif

ifdef DMX
	LIBS+=dmx
endif

ifeq ($(findstring OUTPUT_DDP_PIXEL_MULTI,$(DEFINES)),OUTPUT_DDP_PIXEL_MULTI)
	LIBS+=ws28xxdmx ws28xx jamstapl
else
	ifeq ($(findstring OUTPUT_DMX_PIXEL_MULTI,$(DEFINES)),OUTPUT_DMX_PIXEL_MULTI)
		LIBS+=ws28xxdmx ws28xx jamstapl
	else
		ifeq ($(findstring OUTPUT_DMX_PIXEL,$(DEFINES)),OUTPUT_DMX_PIXEL)
			LIBS+=ws28xxdmx ws28xx tlc59711dmx tlc59711
		endif
	endif
endif

ifeq ($(findstring OUTPUT_DMX_STEPPER,$(DEFINES)),OUTPUT_DMX_STEPPER)
	LIBS+=l6470dmx l6470
endif

ifeq ($(findstring OUTPUT_DMX_TLC59711,$(DEFINES)),OUTPUT_DMX_TLC59711)
	LIBS+=tlc59711dmx tlc59711
endif

ifeq ($(findstring OUTPUT_DMX_ARTNET,$(DEFINES)),OUTPUT_DMX_ARTNET)
	ifneq ($(findstring artnet,$(LIBS)),artnet)
		LIBS+=artnet
	endif
endif

ifeq ($(findstring OUTPUT_DMX_SERIAL,$(DEFINES)),OUTPUT_DMX_SERIAL)
	LIBS+=dmxserial
endif

LIBS+=configstore

ifdef LINUX 
else
	LIBS+=flashcodeinstall flashcode flash
endif	

ifeq ($(findstring NODE_LTC_SMPTE,$(DEFINES)),NODE_LTC_SMPTE)
	DEFINES+=CONFIG_DISPLAY_ENABLE_SSD1311 CONFIG_DISPLAY_ENABLE_HD44780 CONFIG_DISPLAY_ENABLE_CURSOR_MODE
endif

ifneq ($(findstring network,$(LIBS)),network)
	LIBS+=network
endif

ifeq ($(findstring DISPLAY_UDF,$(DEFINES)),DISPLAY_UDF)
	LIBS+=displayudf
endif

ifneq ($(findstring CONFIG_DISPLAY_USE_CUSTOM,$(DEFINES)),CONFIG_DISPLAY_USE_CUSTOM)
	LIBS+=display
else
	LIBS+=$(CONFIG_DISPLAY_LIB)
endif

ifneq ($(findstring properties,$(LIBS)),properties)
	LIBS+=properties
endif

LIBS+=lightset device hal

$(info $$LIBS [${LIBS}])
$(info $$DEFINES [${DEFINES}])