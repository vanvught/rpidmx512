$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)), NO_EMAC)
	else
		EXTRA_SRCDIR+=src/emac
		EXTRA_INCLUDES+=../lib-network/include
	endif
		ifeq ($(findstring ARTNET_HAVE_DMXIN,$(MAKE_FLAGS)), ARTNET_HAVE_DMXIN)
		EXTRA_SRCDIR+=src/artnet
	endif
		ifeq ($(findstring E131_HAVE_DMXIN,$(MAKE_FLAGS)), E131_HAVE_DMXIN)
		EXTRA_SRCDIR+=src/e131
	endif
else
	EXTRA_SRCDIR+=src/emac
	EXTRA_SRCDIR+=src/artnet
	EXTRA_SRCDIR+=src/e131
	EXTRA_INCLUDES+=../lib-network/include
endif
