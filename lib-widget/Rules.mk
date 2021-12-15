DEFINES+=HAVE_FLASHROM
DEFINES+=NO_HDMI_OUTPUT

EXTRA_SRCDIR=src/flashrom src/nohdmi

EXTRA_INCLUDES=../lib-hal/include ../lib-spiflashstore/include ../lib-dmx/include ../lib-rdm/include ../lib-usb/include ../lib-properties/include