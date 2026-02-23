ifneq ($(MAKE_FLAGS),)
else
	DEFINES+=CONFIG_DMXNODE_PIXEL_MAX_PORTS=8
	DEFINES+=DMXNODE_PORTS=32
endif

EXTRA_INCLUDES=../lib-dmx/include  ../lib-network/include ../lib-pixel/include ../lib-pixeldmx/include
