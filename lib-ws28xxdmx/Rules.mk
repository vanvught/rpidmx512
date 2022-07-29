EXTRA_INCLUDES+=../lib-ws28xx/include
EXTRA_INCLUDES+=../lib-lightset/include
EXTRA_INCLUDES+=../lib-properties/include 

EXTRA_SRCDIR+=src/params

ifneq ($(MAKE_FLAGS),)
else
	DEFINES+=CONFIG_PIXELDMX_MAX_PORTS=8
endif