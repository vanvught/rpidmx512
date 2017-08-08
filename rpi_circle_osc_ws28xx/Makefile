#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= oscserver.o oscws28xx.o main.o kernel.o

INCLUDE	+= -I ./include
INCLUDE	+= -I ../lib-osc/include -I ../rpi_circle_libws28xx/include  -I ../lib-network/include 

LIBS = ../lib-osc/libosc.a ../rpi_circle_libws28xx/libws28xx.a ../lib-network/libnetwork.a

LIBS +=	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	$(CIRCLEHOME)/addon/Properties/libproperties.a \
	$(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	$(CIRCLEHOME)/lib/fs/libfs.a \
	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/lib/net/libnet.a \
	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/sample/Rules.mk
