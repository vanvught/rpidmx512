DEFINES =NODE_OSC_SERVER LIGHTSET_PORTS=1
DEFINES+=NODE_RDMNET_LLRP_ONLY 

DEFINES+=OUTPUT_DMX_MONITOR

DEFINES+=DISPLAY_UDF 

#DEFINES+=NDEBUG

SRCDIR=src lib

LIBS=

include ../firmware-template-linux/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
	