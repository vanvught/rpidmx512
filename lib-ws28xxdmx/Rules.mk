EXTRA_INCLUDES+=../lib-ws28xx/include
EXTRA_INCLUDES+=
EXTRA_INCLUDES+=../lib-network/include
EXTRA_INCLUDES+=../lib-properties/include 

EXTRA_SRCDIR+=src/pixeldmxparams

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/pixeldmx
	endif
	ifneq (,$(findstring OUTPUT_DMX_PIXEL_MULTI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/pixeldmxmulti
	endif
	ifeq ($(findstring OUTPUT_DMX_SEND,$(MAKE_FLAGS)), OUTPUT_DMX_SEND)
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	ifneq (,$(findstring CONFIG_RDM_ENABLE_MANUFACTURER_PIDS,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdm/include
		EXTRA_SRCDIR+=src/rdm
	endif
else
	DEFINES+=CONFIG_DMXNODE_PIXEL_MAX_PORTS=8
	DEFINES+=DMXNODE_PORTS=32
	DEFINES+=OUTPUT_DMX_PIXEL OUTPUT_DMX_PIXEL_MULTI
	DEFINES+=CONFIG_RDM_ENABLE_MANUFACTURER_PIDS CONFIG_RDM_MANUFACTURER_PIDS_SET
	EXTRA_INCLUDES+=../lib-dmx/include ../lib-rdm/include
	EXTRA_SRCDIR+=src/pixeldmx src/pixeldmxmulti
	EXTRA_SRCDIR+=src/rdm
endif