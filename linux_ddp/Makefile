DEFINES =NODE_DDP_DISPLAY 
DEFINES+=NODE_RDMNET_LLRP_ONLY
	
DEFINES+=DMXNODE_PORTS=32
DEFINES+=CONFIG_DMXNODE_PIXEL_MAX_PORTS=8 
	
DEFINES+=OUTPUT_DMX_MONITOR

DEFINES+=DISPLAY_UDF

DEFINES+=NDEBUG

SRCDIR=src lib

LIBS=

include ../firmware-template-linux/Rules.mk

prerequisites:
	@echo "constexpr uint32_t DEVICE_SOFTWARE_VERSION_ID="$(shell date "+%s")";" > ./include/software_version_id.h
