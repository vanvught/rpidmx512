
ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_SRCDIR+=src/node src/json
	endif

	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)),NODE_ARTNET)
		ifeq ($(findstring ARTNET_VERSION=4,$(MAKE_FLAGS)),ARTNET_VERSION=4)
			EXTRA_SRCDIR+=src/node src/json
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
else
  EXTRA_SRCDIR+=src/node src/node/dmxin src/controller
  DEFINES+=NODE_E131
  DEFINES+=E131_HAVE_DMXIN
  DEFINES+=OUTPUT_HAVE_STYLESWITCH
  DEFINES+=OUTPUT_DMX_SEND
  DEFINES+=DMXNODE_PORTS=4
  EXTRA_INCLUDES+=../lib-dmx/include
  EXTRA_INCLUDES+=../lib-pixel/include ../lib-pixeldmx/include
  EXTRA_INCLUDES+=../lib-l6470/include ../lib-l6470dmx/include
  EXTRA_INCLUDES+=../lib-tlc59711/include ../lib-tlc59711dmx/include
  DEFINES+=NODE_SHOWFILE 
  DEFINES+=CONFIG_SHOWFILE_PROTOCOL_NODE_E131
endif