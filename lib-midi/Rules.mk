EXTRA_INCLUDES+=../lib-properties/include ../lib-network/include

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)),NO_EMAC)
	else
		EXTRA_SRCDIR+=src/net
	endif
else
	EXTRA_SRCDIR+=src/net
endif