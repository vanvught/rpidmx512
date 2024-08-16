EXTRA_INCLUDES=../lib-rdmsensor/include ../lib-rdmsubdevice/include ../lib-dmx/include ../lib-properties/include ../lib-lightset/include
EXTRA_INCLUDES+=../lib-network/include ../lib-display/include
EXTRA_INCLUDES+=../lib-e131/include

ifneq ($(MAKE_FLAGS),)
	ifeq (,$(findstring NODE_ARTNET,$(MAKE_FLAGS)))
  	ifeq ($(findstring RDM_RESPONDER,$(MAKE_FLAGS)), RDM_RESPONDER)
  		EXTRA_SRCDIR+=src/responder
  		EXTRA_INCLUDES+=../lib-dmxreceiver/include
  	endif
	endif
	
	ifeq ($(findstring RDM_CONTROLLER,$(MAKE_FLAGS)), RDM_CONTROLLER)
 		EXTRA_SRCDIR+=src/controller
	endif
	
	ifneq (,$(findstring NODE_RDMNET_LLRP_ONLY,$(MAKE_FLAGS)))
 		EXTRA_SRCDIR+=src/llrp
 		EXTRA_SRCDIR+=src/rdmnet
	endif
	ifneq (,$(findstring CONFIG_STORE_USE_ROM,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-flashcode/include
	endif
else
	ifneq (, $(shell test -d '../lib-network/src/noemac' && echo -n yes))
		DEFINES+=NO_EMAC
	else
		EXTRA_SRCDIR+=src/llrp src/rdmnet
	endif
	
	ifneq (, $(shell test -d '../lib-dmxreceiver' && echo -n yes))
		EXTRA_INCLUDES+=../lib-dmxreceiver/include
		EXTRA_SRCDIR+=src/responder
	endif	
	DEFINES+=CONFIG_RDM_ENABLE_SUBDEVICES CONFIG_RDM_SUBDEVICES_USE_I2C CONFIG_RDM_SUBDEVICES_USE_SPI
	DEFINES+=CONFIG_RDM_ENABLE_MANUFACTURER_PIDS CONFIG_RDM_MANUFACTURER_PIDS_SET
	DEFINES+=RDM_RESPONDER
endif