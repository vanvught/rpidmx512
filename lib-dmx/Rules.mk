$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)), NO_EMAC)
	else
		EXTRA_SRCDIR+=src/emac
		EXTRA_INCLUDES+=../lib-network/include
	endif
else
	EXTRA_SRCDIR+=src/emac
	EXTRA_INCLUDES+=../lib-network/include
endif
