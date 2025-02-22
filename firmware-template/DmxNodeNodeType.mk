$(info "DmxNodeNodeType.mk")
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
$(info $$DEFINES [${DEFINES}])

FLAGS:=$(MAKE_FLAGS)
ifeq ($(FLAGS),)
	FLAGS:=$(DEFINES)
endif

ifeq ($(findstring NODE_ARTNET,$(FLAGS)), NODE_ARTNET)
	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include ../lib-dmx/include ../lib-rdm/include
endif

ifeq ($(findstring NODE_E131,$(FLAGS)), NODE_E131)
	EXTRA_INCLUDES+=../lib-artnet/include ../lib-e131/include ../lib-dmx/include ../lib-rdm/include
endif

$(info $$EXTRA_INCLUDES [${EXTRA_INCLUDES}])