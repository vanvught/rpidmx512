#DEFINES+=CONFIG_PP_16BITSTUFF

ifneq ($(MAKE_FLAGS),)
else
	DEFINES+=CONFIG_PP_MAX_PORTS=8
	DEFINES+=LIGHTSET_PORTS=32
endif

EXTRA_INCLUDES =../lib-properties/include ../lib-hal/include ../lib-network/include 
EXTRA_INCLUDES+=../lib-lightset/include
