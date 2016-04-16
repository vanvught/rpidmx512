#
# Makefile
#

CIRCLEHOME = ../Circle

OBJS = src/osc.o src/oscaddress.o src/oscblob.o src/oscmessage.o src/oscsend.o src/oscstring.o src/oscutil.o src/pattern_match.o src/udpclient.o src/oscserver.o

EXTRACLEAN = src/*.o

libosc.a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk
