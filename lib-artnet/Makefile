#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS	= src/artnetnode.o  src/blinktask.o src/lightset.o

EXTRACLEAN = src/*.o

libartnet.a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk
