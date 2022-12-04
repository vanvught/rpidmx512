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
	
	ifeq ($(findstring ARTNET_HAVE_FAILSAFE_RECORD,$(MAKE_FLAGS)), ARTNET_HAVE_FAILSAFE_RECORD)
		EXTRA_SRCDIR+=src/node/failsafe
		EXTRA_INCLUDES+=src/node/failsafe
	endif
else
	EXTRA_SRCDIR+=src/node src/node/failsafe src/controller
	EXTRA_INCLUDES+=src/node/failsafe
	DEFINES+=LIGHTSET_PORTS=4
endif