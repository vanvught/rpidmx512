#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= main.o kernel.o

LIBS	= ../libartnet/libartnet.a ../rpi_circle_libdmx/libdmx.a \
	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	$(CIRCLEHOME)/addon/Properties/libproperties.a \
	$(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	$(CIRCLEHOME)/lib/fs/libfs.a \
	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/lib/net/libnet.a \
	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/sample/Rules.mk
