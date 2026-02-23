$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
$(info $$DEFINES [${DEFINES}])

EXTRA_INCLUDES+=../lib-display/include

COND=

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring ESP8266,$(MAKE_FLAGS)), ESP8266)
		EXTRA_SRCDIR+=src/esp8266 src/json
		COND=1
	endif
	ifndef COND
		ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)), NO_EMAC)
			EXTRA_SRCDIR+=src/noemac
			COND=1
		endif
	endif
	ifndef COND
		EXTRA_SRCDIR+=src/net src/core src/iface src/core/ipv4
		EXTRA_SRCDIR+=src/apps/mdns src/apps/ntp src/apps/tftp
		EXTRA_INCLUDES+=config
		EXTRA_SRCDIR+=src/emac src/emac/phy
		EXTRA_SRCDIR+=src/json	
		PHY=
		ifeq ($(findstring DP83848,$(ENET_PHY)), DP83848)
			EXTRA_SRCDIR+=src/emac/phy/dp83848
			PHY=1
		endif
		ifeq ($(findstring LAN8700,$(ENET_PHY)), LAN8700)
			EXTRA_SRCDIR+=src/emac/phy/lan8700
			PHY=1
		endif
		ifeq ($(findstring RTL8201F,$(ENET_PHY)), RTL8201F)
			EXTRA_SRCDIR+=src/emac/phy/rtl8201f
			PHY=1
		endif
		ifndef PHY
			EXTRA_SRCDIR+=src/emac/phy/phygen
		endif
	endif
endif
