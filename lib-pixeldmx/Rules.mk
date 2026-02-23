EXTRA_INCLUDES+=../lib-pixel/include
EXTRA_INCLUDES+=../lib-dmxled/include
EXTRA_INCLUDES+=../lib-displayudf/include ../lib-display/include
EXTRA_INCLUDES+=../lib-network/include

EXTRA_SRCDIR+=src/json

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)), OUTPUT_DMX_SEND)
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	ifneq (,$(findstring CONFIG_RDM_ENABLE_MANUFACTURER_PIDS,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdm/include
		EXTRA_SRCDIR+=src/pixeldmxrdm
	endif
	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
		EXTRA_SRCDIR+=src/pixeldmxparams
		EXTRA_INCLUDES+=../lib-rdm/include
	endif
	ifeq ($(findstring CONFIG_RDM_ENABLE_SENSORS,$(MAKE_FLAGS)), CONFIG_RDM_ENABLE_SENSORS)
		EXTRA_INCLUDES+=../lib-rdmsensor/include
	endif
else
	DEFINES+=CONFIG_DMXNODE_PIXEL_MAX_PORTS=8
	DEFINES+=DMXNODE_PORTS=32
	DEFINES+=OUTPUT_DMX_PIXEL OUTPUT_DMX_PIXEL_MULTI
	DEFINES+=CONFIG_RDM_ENABLE_MANUFACTURER_PIDS CONFIG_RDM_MANUFACTURER_PIDS_SET
	EXTRA_INCLUDES+=../lib-dmx/include ../lib-rdm/include
	EXTRA_SRCDIR+=src/pixeldmxrdm
endif