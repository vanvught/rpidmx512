#
DEFINES = ENABLE_MMU NDEBUG
#
LIBS = dmxreceiver rdmresponder rdm rdmsensor rdmsubdevice dmx ws28xxdmx ws28xx tlc59711dmx tlc59711 monitor ledblink lightset network
#
SRCDIR = firmware lib

include ../firmware-template/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh