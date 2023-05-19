ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_ARTNET,$(MAKE_FLAGS)), NODE_ARTNET)
		EXTRA_SRCDIR+=src/node
	endif
	
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(MAKE_FLAGS)), ARTNET_HAVE_DMXIN)
		EXTRA_SRCDIR+=src/node/dmxin
	endif
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
		EXTRA_SRCDIR+=src/node/rdm
	endif
	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
		EXTRA_SRCDIR+=src/node/rdm
	endif
	ifeq ($(findstring ARTNET_HAVE_TIMECODE,$(MAKE_FLAGS)), ARTNET_HAVE_TIMECODE)
		EXTRA_SRCDIR+=src/node/timecode
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
	EXTRA_SRCDIR+=src/node src/node/failsafe src/node/dmxin src/node/timecode
	EXTRA_SRCDIR+=src/controller
	EXTRA_INCLUDES+=src/node/failsafe
	DEFINES+=ARTNET_HAVE_DMXIN
	DEFINES+=ARTNET_HAVE_TIMECODE
	DEFINES+=ARTNET_HAVE_FAILSAFE_RECORD
	DEFINES+=RDM_CONTROLLER
	DEFINES+=LIGHTSET_PORTS=4
endif