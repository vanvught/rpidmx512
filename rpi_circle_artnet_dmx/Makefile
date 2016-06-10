#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= spisend.o main.o kernel.o

INCLUDE	+= -I ../rpi_circle_libdmx/include -I ../rpi_circle_libws28xx/include -I ../lib-artnet/include

LIBS  = ../rpi_circle_libdmx/libdmx.a ../rpi_circle_libws28xx/libws28xx.a ../lib-artnet/libartnet.a 

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
