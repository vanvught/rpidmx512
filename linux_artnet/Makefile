#
DEFINES= ARTNET_NODE ARTNET4_NODE DMX_MONITOR ENABLE_SPIFLASH #NDEBUG
#
LIBS=dmxmonitor rdmresponder rdm rdmsensor rdmsubdevice artnet4 artnet artnethandlers e131 lightset
#
SRCDIR= src lib

include ../linux-template/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
