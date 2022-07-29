ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring NODE_E131,$(MAKE_FLAGS)), NODE_E131)
		EXTRA_SRCDIR+=src/node
	endif
	ifeq ($(findstring E131_CONTROLLER,$(MAKE_FLAGS)), E131_CONTROLLER)
		EXTRA_SRCDIR+=src/controller
	endif
	ifeq ($(findstring NODE_NODE,$(MAKE_FLAGS)), NODE_NODE)
		EXTRA_SRCDIR+=src/node
	endif
else
EXTRA_SRCDIR+=src/node src/controller
DEFINES+=LIGHTSET_PORTS=4
endif