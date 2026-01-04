$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=../lib-network/include ../lib-display/include 
EXTRA_SRCDIR+=src/json

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring ENABLE_HTTPD,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/httpd src/http
	endif
	ifneq (,$(findstring ENABLE_TFTP_SERVER,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-flashcode/include ../lib-flashcodeinstall/include
	endif	
else
	EXTRA_SRCDIR+=src/httpd src/httpd/http
	EXTRA_INCLUDES+=../lib-flashcode/include ../lib-flashcodeinstall/include
	DEFINES+=ENABLE_CONTENT
	DEFINES+=ARTNET_VERSION=4
	DEFINES+=RDM_CONTROLLER ENABLE_NET_PHYSTATUS CONFIG_USB_HOST_MSC ENABLE_PHY_SWITCH
	DEFINES+=NODE_SHOWFILE CONFIG_SHOWFILE_FORMAT_OLA CONFIG_SHOWFILE_PROTOCOL_E131
	DEFINES+=DEBUG_HTTPD
endif
