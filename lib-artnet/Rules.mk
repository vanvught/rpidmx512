EXTRA_INCLUDES+=../lib-network/include

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_SRCDIR+=src/node src/json
	endif
	
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(MAKE_FLAGS)), ARTNET_HAVE_DMXIN)
		EXTRA_SRCDIR+=src/node/dmxin
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
		EXTRA_SRCDIR+=src/node/rdm
		EXTRA_SRCDIR+=src/node/rdm/controller
		EXTRA_INCLUDES+=../lib-rdm/include ../lib-dmx/include
		ifeq ($(findstring ENABLE_HTTPD,$(MAKE_FLAGS)), ENABLE_HTTPD)
			EXTRA_SRCDIR+=src/node/rdm/controller/json
		endif
	endif
	
	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
		EXTRA_SRCDIR+=src/node/rdm
		EXTRA_SRCDIR+=src/node/rdm/responder
		EXTRA_INCLUDES+=../lib-rdm/include ../lib-rdmsensor/include
	endif
	
	ifeq ($(findstring ARTNET_CONTROLLER,$(MAKE_FLAGS)), ARTNET_CONTROLLER)
		EXTRA_SRCDIR+=src/controller
	endif
		
	ifeq ($(findstring ARTNET_HAVE_FAILSAFE_RECORD,$(MAKE_FLAGS)), ARTNET_HAVE_FAILSAFE_RECORD)
		EXTRA_SRCDIR+=src/node/failsafe
		EXTRA_INCLUDES+=src/node/failsafe
	endif
			
	ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)), ARTNET_VERSION=4)
		EXTRA_INCLUDES+=../lib-e131/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)), OUTPUT_DMX_SEND)
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)), OUTPUT_DMX_PIXEL)
		EXTRA_INCLUDES+=../lib-pixel/include ../lib-pixeldmx/include
	endif

	ifeq ($(findstring OUTPUT_DMX_SERIAL,$(MAKE_FLAGS)), OUTPUT_DMX_SERIAL)
		EXTRA_INCLUDES+=../lib-dmxserial/include
	endif
	
	ifeq ($(findstring OUTPUT_DMX_STEPPER,$(MAKE_FLAGS)), OUTPUT_DMX_STEPPER)
		EXTRA_INCLUDES+=../lib-l6470/include ../lib-l6470dmx/include
		EXTRA_INCLUDES+=../lib-tlc59711/include ../lib-tlc59711dmx/include
	endif

	ifeq ($(findstring OUTPUT_DMX_PCA9685,$(MAKE_FLAGS)), OUTPUT_DMX_PCA9685)
		EXTRA_INCLUDES+=../lib-pca9685/include ../lib-pca9685dmx/include
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
	EXTRA_INCLUDES+=../lib-pixel/include ../lib-pixeldmx/include
	EXTRA_INCLUDES+=../lib-l6470/include ../lib-l6470dmx/include
	EXTRA_INCLUDES+=../lib-tlc59711/include ../lib-tlc59711dmx/include
	DEFINES+=ARTNET_HAVE_TIMECODE
	DEFINES+=ARTNET_HAVE_FAILSAFE_RECORD
	DEFINES+=ARTNET_HAVE_DMXIN E131_HAVE_DMXIN
	DEFINES+=OUTPUT_HAVE_STYLESWITCH
	DEFINES+=OUTPUT_DMX_SEND
	DEFINES+=ARTNET_ENABLE_SENDDIAG
	DEFINES+=NODE_ARTNET
	DEFINES+=RDM_CONTROLLER
	DEFINES+=ARTNET_VERSION=4
	DEFINES+=DMXNODE_PORTS=4
	DEFINES+=NODE_SHOWFILE 
	DEFINES+=CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET
endif