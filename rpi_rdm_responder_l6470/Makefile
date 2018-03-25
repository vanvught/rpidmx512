#
DEFINES = ENABLE_MMU NDEBUG
#
LIBS = dmxreceiver rdmresponder rdm rdmsensor rdmsubdevice dmx l6470dmx l6470 pwmdmx pca9685dmx pca9685 tlc59711dmx tlc59711 ledblink lightset c++ 
#
SRCDIR = firmware lib

include ../firmware-template/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh