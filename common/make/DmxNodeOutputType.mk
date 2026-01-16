$(info "DmxNodeOutputType.mk")
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
$(info $$DEFINES [${DEFINES}])

FLAGS:=$(MAKE_FLAGS)
ifeq ($(FLAGS),)
	FLAGS:=$(DEFINES)
endif

EXTRA_INCLUDES+=../lib-dmxnode/include

ifeq ($(findstring OUTPUT_DMX_ARTNET,$(FLAGS)), OUTPUT_DMX_ARTNET)
	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include
endif

ifeq ($(findstring OUTPUT_DMX_SEND,$(FLAGS)), OUTPUT_DMX_SEND)
	EXTRA_INCLUDES+=../lib-dmx/include ../lib-network/include
endif

ifeq ($(findstring OUTPUT_DMX_PIXEL,$(FLAGS)), OUTPUT_DMX_PIXEL)
	EXTRA_INCLUDES+=../lib-pixel/include ../lib-pixeldmx/include
endif

ifeq ($(findstring OUTPUT_DMX_MONITOR,$(MAKE_FLAGS)), OUTPUT_DMX_MONITOR)
	EXTRA_INCLUDES+=../lib-dmxmonitor/include
endif

ifeq ($(findstring OUTPUT_DMX_NULL,$(MAKE_FLAGS)), OUTPUT_DMX_NULL)
	EXTRA_INCLUDES+=../lib-rdm/include ../lib-e131/include
endif

ifeq ($(findstring OUTPUT_DMX_SHOWFILE,$(MAKE_FLAGS)), OUTPUT_DMX_SHOWFILE)
	EXTRA_INCLUDES+=../lib-showfile/include
endif
	
ifeq ($(findstring OUTPUT_DMX_SERIAL,$(FLAGS)), OUTPUT_DMX_SERIAL)
	EXTRA_INCLUDES+=../lib-dmxserial/include ../lib-network/include
endif
	
ifeq ($(findstring OUTPUT_DMX_STEPPER,$(FLAGS)), OUTPUT_DMX_STEPPER)
	EXTRA_INCLUDES+=../lib-l6470/include ../lib-l6470dmx/include
	EXTRA_INCLUDES+=../lib-tlc59711/include ../lib-tlc59711dmx/include
endif
	
ifeq ($(findstring OUTPUT_DMX_PCA9685,$(FLAGS)), OUTPUT_DMX_PCA9685)
	EXTRA_INCLUDES+=../lib-pca9685/include ../lib-pca9685dmx/include
endif

$(info $$EXTRA_INCLUDES [${EXTRA_INCLUDES}])