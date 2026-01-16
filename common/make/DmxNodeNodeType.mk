$(info "DmxNodeNodeType.mk")
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
$(info $$DEFINES [${DEFINES}])

FLAGS:=$(MAKE_FLAGS)
ifeq ($(FLAGS),)
	FLAGS:=$(DEFINES)
endif

ifeq ($(findstring NODE_ARTNET,$(FLAGS)), NODE_ARTNET)
	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include ../lib-rdm/include
	ifeq ($(findstring CONFIG_RDM_ENABLE_SENSORS,$(FLAGS)), CONFIG_RDM_ENABLE_SENSORS)
		EXTRA_INCLUDES+=../lib-rdmsensor/include
	endif
endif

ifeq ($(findstring NODE_E131,$(FLAGS)), NODE_E131)
	EXTRA_INCLUDES+=../lib-e131/include
endif

ifeq ($(findstring NODE_DDP_DISPLAY,$(FLAGS)), NODE_DDP_DISPLAY)
	EXTRA_INCLUDES+=../lib-ddp/include
endif

ifeq ($(findstring NODE_PP,$(FLAGS)), NODE_PP)
	EXTRA_INCLUDES+=../lib-pp/include
endif

EXTRA_INCLUDES:= $(strip $(sort $(EXTRA_INCLUDES)))
$(info $$EXTRA_INCLUDES [${EXTRA_INCLUDES}])