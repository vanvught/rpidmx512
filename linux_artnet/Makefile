DEFINES=NODE_ARTNET OUTPUT_DMX_MONITOR ENABLE_SPIFLASH #NDEBUG
LIBS=dmxmonitor rdmresponder rdm rdmsensor rdmsubdevice rdmnet artnet4 artnet e131 lightset
SRCDIR=src lib

include ../firmware-template-linux/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
