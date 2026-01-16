EXTRA_INCLUDES+=../lib-network/include
EXTRA_SRCDIR+=src/json

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)),NO_EMAC)
	else
		EXTRA_SRCDIR+=src/net
	endif
else
	EXTRA_SRCDIR+=src/net
endif