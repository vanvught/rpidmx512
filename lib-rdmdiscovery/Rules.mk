EXTRA_INCLUDES+=../lib-rdm/include ../lib-lightset/include ../lib-hal/include ../lib-network/include

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/artnet
	endif
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_INCLUDES+=../lib-artnet/include
		EXTRA_SRCDIR+=src/artnet
	endif
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
		EXTRA_INCLUDES+=
		EXTRA_SRCDIR+=src/rdm
	endif
else
	EXTRA_INCLUDES+=../lib-artnet/include
	EXTRA_SRCDIR+=src/rdm src/artnet
	DEFINES+=LIGHTSET_PORTS=4
endif