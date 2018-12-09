#
DEFINES= ENABLE_SPIFLASH NDEBUG
#
LIBS= dmxmonitor rdmresponder rdm rdmsensor rdmsubdevice artnet properties lightset ledblink debug e131
#
SRCDIR= src lib

include ../linux-template/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh
