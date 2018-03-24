#
DEFINES = RASPPI NDEBUG
#
LIBS = rdmresponder rdm rdmsensor rdmsubdevice artnet l6470dmx l6470 pwmdmx pca9685dmx pca9685 tlc59711dmx tlc59711 ledblink lightset
LIBS += bob i2c bcm2835_raspbian
#
SRCDIR = src lib

include ../linux-template/Rules.mk

prerequisites:
	./generate_sofware_version_id.sh