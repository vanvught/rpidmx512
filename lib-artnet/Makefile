#
# Makefile
#

CIRCLEHOME = ../Circle

INCLUDE	+= -I ../lib-artnet/include

OBJS	= src/artnetnode.o  src/blinktask.o src/lightset.o

EXTRACLEAN = src/*.o

libartnet.a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)
	$(PREFIX)objdump -D libartnet.a | $(PREFIX)c++filt > libartnet.lst

include $(CIRCLEHOME)/Rules.mk
