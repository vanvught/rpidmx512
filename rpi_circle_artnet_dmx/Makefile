#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= blinktask.o spisend.o main.o kernel.o

INCLUDE	+= -I ./include
INCLUDE	+= -I ../rpi_circle_libdmx/include -I ../rpi_circle_libws28xx/include
INCLUDE	+= -I ../lib-artnet/include -I ../lib-lightset/include -I ../lib-ledblink/include -I ../lib-network/include

LIBS = ../rpi_circle_libdmx/libdmx.a ../rpi_circle_libws28xx/libws28xx.a ../lib-artnet/libartnet.a ../lib-lightset/liblightset.a ../lib-ledblink/libledblink.a ../lib-network/libnetwork.a

LIBS += $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	$(CIRCLEHOME)/addon/Properties/libproperties.a \
	$(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	$(CIRCLEHOME)/lib/fs/libfs.a \
	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/lib/net/libnet.a \
	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/sample/Rules.mk
