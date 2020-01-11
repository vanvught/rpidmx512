#
DEFINES= ARTNET_NODE ARTNET4_NODE ENABLE_SPIFLASH NDEBUG
#
LIBS=dmxmonitor rdmresponder rdm rdmsensor rdmsubdevice artnet4 artnet artnethandlers e131 properties lightset ledblink debug
#
SRCDIR= src lib

include ../linux-template/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
