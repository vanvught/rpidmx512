EXTRA_INCLUDES+=../lib-ws28xx/include
EXTRA_INCLUDES+=../lib-lightset/include
EXTRA_INCLUDES+=../lib-properties/include 

EXTRA_SRCDIR+=src/params

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring ENABLE_RDM_MANUFACTURER_PIDS,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdm/include
		EXTRA_SRCDIR+=src/rdm
	endif
else
	DEFINES+=CONFIG_PIXELDMX_MAX_PORTS=8
	DEFINES+=ENABLE_RDM_MANUFACTURER_PIDS
	DEFINES+=OUTPUT_DMX_PIXEL
	EXTRA_INCLUDES+=../lib-rdm/include
	EXTRA_SRCDIR+=src/rdm
endif