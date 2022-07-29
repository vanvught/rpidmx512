ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_SRCDIR+=src/node
	endif
	ifeq ($(findstring ARTNET_CONTROLLER,$(MAKE_FLAGS)), ARTNET_CONTROLLER)
		EXTRA_SRCDIR+=src/controller
	endif
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_SRCDIR+=src/node
	endif
else
EXTRA_SRCDIR+=src/node src/controller
DEFINES+=LIGHTSET_PORTS=4
endif