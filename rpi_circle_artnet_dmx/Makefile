#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= main.o kernel.o

INCLUDE	+= -I ./include
INCLUDE	+= -I ../lib-artnet/include -I ../lib-lightset/include -I ../lib-ledblink/include
INCLUDE	+= -I ../lib-dmxsend/include
INCLUDE	+= -I ../lib-ws28xxdmx/include -I ../lib-ws28xx/include
INCLUDE	+= -I ../lib-hal/include -I ../lib-network/include
INCLUDE	+= -I ../include 

LIBS = ../lib-dmxsend/libdmxsend.a ../lib-ws28xxdmx/libws28xxdmx.a ../lib-ws28xx/libws28xx.a ../lib-artnet/libartnet.a ../lib-lightset/liblightset.a ../lib-ledblink/libledblink.a  ../lib-network/libnetwork.a ../lib-hal/libhal.a ../lib-properties/libproperties.a ../lib-utils/libutils.a

LIBS += $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	$(CIRCLEHOME)/lib/fs/libfs.a \
	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/lib/net/libnet.a \
	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/sample/Rules.mk
