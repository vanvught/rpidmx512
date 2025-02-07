$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
$(info $$DEFINES [${DEFINES}])

EXTRA_INCLUDES+=../lib-properties/include ../lib-display/include

COND=

ifneq ($(MAKE_FLAGS),)
	ifeq ($(findstring ESP8266,$(MAKE_FLAGS)), ESP8266)
		EXTRA_SRCDIR+=src/esp8266 src/params
		COND=1
	endif
	ifndef COND
		ifeq ($(findstring NO_EMAC,$(MAKE_FLAGS)), NO_EMAC)
			EXTRA_SRCDIR+=src/noemac
			COND=1
		endif
	endif
	ifndef COND
		EXTRA_SRCDIR+=src/net src/net/core src/net/netif src/net/core/ipv4
		EXTRA_SRCDIR+=src/net/apps/mdns src/net/apps/ntp src/net/apps/tftp
		EXTRA_INCLUDES+=config src/net
		EXTRA_SRCDIR+=src/emac src/emac/phy
		EXTRA_SRCDIR+=src/params 
		ifeq ($(findstring ENABLE_PHY_SWITCH,$(MAKE_FLAGS)), ENABLE_PHY_SWITCH)
			EXTRA_SRCDIR+=src/emac/dsa
		endif		
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
else
	EXTRA_SRCDIR+=src/net src/net/core src/net/netif src/net/core/ipv4
	EXTRA_SRCDIR+=src/net/apps/mdns src/net/apps/ntp src/net/apps/tftp
	EXTRA_INCLUDES+=config src/net
	EXTRA_SRCDIR+=src/emac src/emac/phy
	EXTRA_SRCDIR+=src/emac/phy/dp83848 src/emac/phy/lan8700 src/emac/phy/phygen src/emac/phy/rtl8201f
	EXTRA_SRCDIR+=src/params
	DEFINES+=RTL8201F_LED1_LINK_ALL
	DEFINES+=TCP_TX_QUEUE_SIZE=2
	DEFINES+=DEBUG_NET_TCP
endif
