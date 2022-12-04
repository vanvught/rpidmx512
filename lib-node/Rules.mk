EXTRA_INCLUDES =../lib-lightset/include
EXTRA_INCLUDES+=../lib-artnet/include ../lib-artnet4/include ../lib-rdmdiscovery/include ../lib-rdm/include
EXTRA_INCLUDES+=../lib-e131/include
EXTRA_INCLUDES+=../lib-dmx/include
EXTRA_INCLUDES+=../lib-hal/include ../lib-network/include
EXTRA_INCLUDES+=../lib-properties/include
EXTRA_INCLUDES+=../lib-display/include

EXTRA_INCLUDES+=../lib-configstore/include

EXTRA_SRCDIR+=src/dmx

ifneq ($(MAKE_FLAGS),)
else
DEFINES+=LIGHTSET_PORTS=4
endif