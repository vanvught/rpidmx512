#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= src/dmxsend.o

EXTRACLEAN = src/*.o

libdmx.a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk
