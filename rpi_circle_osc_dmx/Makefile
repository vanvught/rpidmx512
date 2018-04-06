#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= main.o kernel.o 

INCLUDE	+= -I ./include
INCLUDE	+= -I ../lib-oscserver/include -I ../lib-osc/include 
INCLUDE	+= -I ../lib-dmxsend/include -I ../lib-lightset/include
INCLUDE	+= -I ../lib-ledblink/include
INCLUDE	+= -I ../lib-hal/include -I ../lib-network/include
INCLUDE	+= -I ../include 

LIBS = ../lib-oscserver/liboscserver.a ../lib-osc/libosc.a ../lib-dmxsend/libdmxsend.a ../lib-hal/libhal.a ../lib-network/libnetwork.a ../lib-properties/libproperties.a ../lib-lightset/liblightset.a ../lib-ledblink/libledblink.a  ../lib-utils/libutils.a

LIBS +=	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	$(CIRCLEHOME)/lib/fs/libfs.a \
	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/lib/net/libnet.a \
	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/sample/Rules.mk
