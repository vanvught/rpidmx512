PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-i2c/lib_linux
LDLIBS := -li2c
LIBDEP := $(ROOT)/lib-i2c/lib_linux/libi2c.a

INCLUDES := -I$(ROOT)/lib-i2c/include

COPS := -Wall -Werror -O3 -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : i2cdetect

clean :
	rm -f *.o
	rm -f *.lst
	rm -f i2cdetect
	cd $(ROOT)/lib-i2c && make -f Makefile.Linux clean
	
OBJECTS := $(patsubst $(ROOT)/lib-i2c/src/%.cpp,%.o,$(wildcard $(ROOT)/lib-i2c/src/*.cpp))

$(ROOT)/lib-i2c/lib_linux/libi2c.a :
	cd $(ROOT)/lib-i2c && make -f Makefile.Linux "DEF=-DRASPPI"
	
i2cdetect : Makefile i2cdetect.c $(ROOT)/lib-i2c/lib_linux/libi2c.a
	$(CC) i2cdetect.c $(INCLUDES) $(COPS) -o i2cdetect $(LIB) $(LDLIBS)
	$(PREFIX)objdump -D i2cdetect | $(PREFIX)c++filt > i2cdetect.lst
	
