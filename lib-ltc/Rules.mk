$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=../lib-display/include ../lib-artnet/include ../lib-tcnet/include  ../lib-midi/include ../lib-network/include ../lib-properties/include ../lib-lightset/include

ifneq ($(MAKE_FLAGS),)
	COND=
	
	ifneq ($(findstring CONFIG_LTC_DISABLE_RGB_PANEL,$(MAKE_FLAGS)), CONFIG_LTC_DISABLE_RGB_PANEL)
		COND=1
		EXTRA_SRCDIR+=src/displayrgb/panel
		EXTRA_INCLUDES+=../lib-rgbpanel/include
	endif
	
	ifneq ($(findstring CONFIG_LTC_DISABLE_WS28XX,$(MAKE_FLAGS)), CONFIG_LTC_DISABLE_WS28XX)
		COND=1
		EXTRA_SRCDIR+=src/displayrgb/ws28xx
		EXTRA_INCLUDES+=../lib-ws28xxdisplay/include ../lib-ws28xx/include
	endif
	
	ifdef COND
		EXTRA_SRCDIR+=src/displayrgb
	endif
else
	DEFINES+=ARTNET_VERSION=3
	DEFINES+=LIGHTSET_PORTS=1
	DEFINES+=ARTNET_HAVE_TIMECODE
endif