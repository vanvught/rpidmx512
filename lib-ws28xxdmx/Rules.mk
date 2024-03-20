EXTRA_INCLUDES+=../lib-ws28xx/include
EXTRA_INCLUDES+=../lib-lightset/include
EXTRA_INCLUDES+=../lib-properties/include 

EXTRA_SRCDIR+=src/params

ifneq ($(MAKE_FLAGS),)
  ifneq (,$(findstring OUTPUT_DMX_PIXEL,$(MAKE_FLAGS)))
 	 EXTRA_SRCDIR+=src/dmx
  endif
 ifneq (,$(findstring OUTPUT_DMX_PIXEL_MULTI,$(MAKE_FLAGS)))
 	 EXTRA_SRCDIR+=src/dmxmulti
  endif
	ifneq (,$(findstring CONFIG_RDM_ENABLE_MANUFACTURER_PIDS,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdm/include
		EXTRA_SRCDIR+=src/rdm
	endif
else
	DEFINES+=CONFIG_PIXELDMX_MAX_PORTS=8
	DEFINES+=LIGHTSET_PORTS=32
	DEFINES+=CONFIG_RDM_ENABLE_MANUFACTURER_PIDS CONFIG_RDM_MANUFACTURER_PIDS_SET
	DEFINES+=OUTPUT_DMX_PIXEL
	EXTRA_INCLUDES+=../lib-rdm/include
	EXTRA_SRCDIR+=src/dmx src/dmxmulti src/rdm
endif