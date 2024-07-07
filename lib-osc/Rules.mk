$(info [${CURDIR}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=../lib-display/include ../lib-properties/include ../lib-network/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring NODE_OSC_CLIENT,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/client
		DO_INCLUDE=1
	endif
	ifneq (,$(findstring NODE_OSC_SERVER,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/server
		EXTRA_INCLUDES+=../lib-lightset/include
		DO_INCLUDE=1
	endif
else
	EXTRA_SRCDIR+=src/client
	EXTRA_SRCDIR+=src/server
	DO_INCLUDE=1
endif

ifdef DO_INCLUDE
	EXTRA_INCLUDES+=../lib-display/include ../lib-properties/include ../lib-network/include
endif