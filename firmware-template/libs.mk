$(info $$DEFINES [${DEFINES}])

ifeq ($(findstring NO_EMAC,$(DEFINES)),NO_EMAC)
else
	LIBS+=remoteconfig
endif

ifeq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
	LIBS+=artnet4 artnet e131
endif

ifeq ($(findstring NODE_E131,$(DEFINES)),NODE_E131)
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131
	endif
endif

ifeq ($(findstring NODE_SHOWFILE,$(DEFINES)),NODE_SHOWFILE)
	LIBS+=showfile osc
	ifneq ($(findstring artnet,$(LIBS)),artnet)
		LIBS+=artnet
	endif
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

ifeq ($(findstring RDM_CONTROLLER,$(DEFINES)),RDM_CONTROLLER)
	LIBS+=rdmdiscovery rdm
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
	LIBS+=rdm dmx
endif

ifeq ($(findstring NODE_DMX,$(DEFINES)),NODE_DMX)
	LIBS+=dmxreceiver dmx
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
	LIBS+=rdm
endif

ifeq ($(findstring e131,$(LIBS)),e131)
	LIBS+=uuid
endif

ifeq ($(findstring OUTPUT_DMX_MONITOR,$(DEFINES)),OUTPUT_DMX_MONITOR)
	LIBS+=dmxmonitor	
endif

ifeq ($(findstring OUTPUT_DMX_SEND,$(DEFINES)),OUTPUT_DMX_SEND)
	LIBS+=dmxsend dmx
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

ifeq ($(findstring OUTPUT_DDP_PIXEL,$(DEFINES)),OUTPUT_DDP_PIXEL)
	LIBS+=ws28xx
endif

ifeq ($(findstring OUTPUT_DMX_STEPPER,$(DEFINES)),OUTPUT_DMX_STEPPER)
	LIBS+=l6470dmx l6470
endif

ifeq ($(findstring OUTPUT_DMX_TLC59711,$(DEFINES)),OUTPUT_DMX_TLC59711)
	LIBS+=tlc59711dmx tlc59711
endif

ifeq ($(findstring OUTPUT_DMX_ARTNET,$(DEFINES)),OUTPUT_DMX_ARTNET)
	LIBS+=artnet
endif

ifeq ($(findstring OUTPUT_DMX_SERIAL,$(DEFINES)),OUTPUT_DMX_SERIAL)
	LIBS+=dmxserial
endif

ifdef COND
	LIBS+=spiflashinstall spiflashstore spiflash
endif

ifeq ($(findstring NODE_LTC_SMPTE,$(DEFINES)),NODE_LTC_SMPTE)
	DEFINES+=ENABLE_SSD1311 ENABLE_TC1602 ENABLE_CURSOR_MODE
endif

ifeq ($(findstring NO_EMAC,$(DEFINES)),NO_EMAC)
else
	LIBS+=network properties
endif

ifeq ($(findstring DISPLAY_UDF,$(DEFINES)),DISPLAY_UDF)
	LIBS+=displayudf
endif

ifneq ($(findstring network,$(LIBS)),network)
	LIBS+=network
endif

ifneq ($(findstring properties,$(LIBS)),properties)
	LIBS+=properties
endif

LIBS+=lightset display device hal

$(info $$LIBS [${LIBS}])