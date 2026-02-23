$(info [${CURDIR}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=../lib-network/include
EXTRA_SRCDIR+=src/json

ifneq ($(MAKE_FLAGS),)
	ifeq (,$(findstring CONFIG_SHOWFILE_USE_CUSTOM_DISPLAY,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/display
		EXTRA_INCLUDES+=../lib-display/include
	endif
	
	ifneq (,$(findstring CONFIG_SHOWFILE_ENABLE_OSC,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/osc
		EXTRA_INCLUDES+=../lib-osc/include
	endif
	
	ifneq (,$(findstring CONFIG_SHOWFILE_FORMAT_OLA,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/formats/ola
	endif
	
	ifneq (,$(findstring CONFIG_SHOWFILE_PROTOCOL_E131,$(MAKE_FLAGS)))
		E131=1
	endif
	
	ifneq (,$(findstring CONFIG_SHOWFILE_PROTOCOL_NODE_E131,$(MAKE_FLAGS)))
		E131=1
	endif
	
	ifneq (,$(findstring CONFIG_SHOWFILE_PROTOCOL_ARTNET,$(MAKE_FLAGS)))
		ARTNET=1
	endif
	
	ifneq (,$(findstring CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET,$(MAKE_FLAGS)))
		ARTNET=1
	endif
	
	ifdef E131
		EXTRA_INCLUDES+=../lib-e131/include
		EXTRA_INCLUDES+=../lib-dmx/include
	endif
	
	ifdef ARTNET
		EXTRA_INCLUDES+=../lib-dmx/include
		EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include
		ifneq (,$(findstring RDM,$(MAKE_FLAGS)))
			EXTRA_INCLUDES+=../lib-rdm/include ../lib-dmx/include
		endif
	endif
else
	EXTRA_SRCDIR+=src/display
	EXTRA_SRCDIR+=src/formats/ola
	EXTRA_SRCDIR+=src/protocols/artnet
	EXTRA_INCLUDES+=../lib-display/include
	EXTRA_INCLUDES+=../lib-osc/include
	EXTRA_INCLUDES+=../lib-e131/include
	EXTRA_INCLUDES+=../lib-artnet/include
	DEFINES+=NODE_SHOWFILE
	DEFINES+=CONFIG_SHOWFILE_FORMAT_OLA
#	DEFINES+=CONFIG_SHOWFILE_PROTOCOL_E131
	DEFINES+=CONFIG_SHOWFILE_PROTOCOL_NODE_E131
	DEFINES+=CONFIG_SHOWFILE_ENABLE_OSC
	DEFINES+=DEBUG_SHOWFILEOSC
endif

$(info $$EXTRA_SRCDIR [${EXTRA_SRCDIR}])
$(info $$EXTRA_INCLUDES [${EXTRA_INCLUDES}])