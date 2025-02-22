ifneq ($(MAKE_FLAGS),)
else
	DEFINES+=CONFIG_DMXNODE_PIXEL_MAX_PORTS=8
	DEFINES+=DMXNODE_PORTS=32
endif

EXTRA_INCLUDES=../lib-dmx/include  ../lib-properties/include ../lib-network/include ../lib-ws28xx/include ../lib-ws28xxdmx/include
