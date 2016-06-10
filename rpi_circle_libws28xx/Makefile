#
# Makefile
#

CIRCLEHOME = ../Circle

INCLUDE	+= -I ./include

OBJS	= src/ws28xxstripe.o

EXTRACLEAN = src/*.o

libws28xx.a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)
	$(PREFIX)objdump -D libws28xx.a | $(PREFIX)c++filt > libws28xx.lst

include $(CIRCLEHOME)/Rules.mk
