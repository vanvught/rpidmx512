PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-tlc59711/lib_linux
LDLIBS := -ltlc59711 
LIBDEP := $(ROOT)/lib-tlc59711/lib_linux/libtlc59711.a

INCLUDES := -I$(ROOT)/lib-tlc59711/include

COPS := -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : pwmled

clean :
	rm -f *.o
	rm -f *.lst
	rm -f simple pwmled servo
	cd $(ROOT)/lib-tlc59711 && make -f Makefile.Linux clean
	
$(ROOT)/lib-tlc59711/lib_linux/libtlc59711.a :
	cd $(ROOT)/lib-tlc59711 && make -f Makefile.Linux "DEF=-DRASPPI"
	
pwmled : Makefile pwmled.cpp $(ROOT)/lib-tlc59711/lib_linux/libtlc59711.a
	$(CPP) pwmled.cpp $(INCLUDES) $(COPS) -o pwmled $(LIB) $(LDLIBS)
	$(PREFIX)objdump -D pwmled | $(PREFIX)c++filt > pwmled.lst

	