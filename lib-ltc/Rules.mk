$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=../lib-display/include ../lib-artnet/include ../lib-tcnet/include ../lib-midi/include
EXTRA_INCLUDES+=../lib-dmxled/include
EXTRA_INCLUDES+=../lib-network/include
EXTRA_INCLUDES+=

EXTRA_SRCDIR+=src/json

ifneq ($(MAKE_FLAGS),)
	COND=
	
	ifneq ($(findstring CONFIG_LTC_DISABLE_RGB_PANEL,$(MAKE_FLAGS)), CONFIG_LTC_DISABLE_RGB_PANEL)
		COND=1
		EXTRA_SRCDIR+=src/displayrgb/panel
		EXTRA_INCLUDES+=../lib-rgbpanel/include
	endif
	
	ifneq ($(findstring CONFIG_LTC_DISABLE_WS28XX,$(MAKE_FLAGS)), CONFIG_LTC_DISABLE_WS28XX)
		COND=1
		EXTRA_SRCDIR+=src/displayrgb/pixel
		EXTRA_INCLUDES+=../lib-pixeldisplay/include ../lib-pixel/include
	endif
	
	ifdef COND
		EXTRA_SRCDIR+=src/displayrgb
	endif
else
	DEFINES+=ARTNET_VERSION=3
	DEFINES+=DMXNODE_PORTS=1
	DEFINES+=OUTPUT_DMX_NULL
	DEFINES+=ARTNET_HAVE_TIMECODE
endif