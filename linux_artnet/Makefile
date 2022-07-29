DEFINES=NODE_ARTNET RDM_RESPONDER 
DEFINES+=OUTPUT_DMX_MONITOR DISPLAY_UDF ENABLE_SPIFLASH 
DEFINES+=#NDEBUG

SRCDIR=src lib

LIBS=

include ../firmware-template-linux/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
