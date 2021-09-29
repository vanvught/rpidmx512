DEFINES=NODE_ARTNET OUTPUT_DMX_MONITOR DISPLAY_UDF ENABLE_SPIFLASH #NDEBUG
LIBS=dmxmonitor rdmresponder rdm rdmsensor rdmsubdevice rdmnet artnet4 artnet e131 lightset displayudf
SRCDIR=src lib

include ../firmware-template-linux/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
