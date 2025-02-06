$(info "Common.mk")
$(info $$DEFINES [${DEFINES}])

DEFINES+=-DPHY_GENERIC=255
DEFINES+=-DPHY_TYPE=PHY_GENERIC
DEFINES+=-DENABLE_TFTP_SERVER
DEFINES+=-DCONFIG_MDNS_DOMAIN_REVERSE
DEFINES+=-DISABLE_INTERNAL_RTC
DEFINES+=-D__FPU_PRESENT=1 -D__GIC_PRESENT=1
DEFINES+=-DHTTPD_CONTENT_SIZE=4096
DEFINES+=-DCONFIG_EMAC_HASH_MULTICAST_FILTER

$(info $$DEFINES [${DEFINES}])
