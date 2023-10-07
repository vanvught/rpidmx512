EXTRA_INCLUDES+=../lib-tlc59711/include
EXTRA_INCLUDES+=../lib-lightset/include
EXTRA_INCLUDES+=../lib-properties/include 

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring ENABLE_RDM_MANUFACTURER_PIDS,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-rdm/include
		EXTRA_SRCDIR+=src/rdm
	endif
else
	DEFINES+=OUTPUT_DMX_TLC59711
	DEFINES+=ENABLE_RDM_MANUFACTURER_PIDS
	EXTRA_INCLUDES+=../lib-rdm/include
	EXTRA_SRCDIR+=src/rdm
endif