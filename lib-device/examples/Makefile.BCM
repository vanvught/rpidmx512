PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-device/lib_linux
LDLIBS := -ldevice
LIBDEP := $(ROOT)/lib-device/lib_linux/libdevice.a

INCLUDES := -I$(ROOT)/lib-device/include -I$(ROOT)/lib-hal/include

COPS := -Wall -Werror -O2 -fno-rtti -std=c++11 -DRASPPI -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : max7219matrix 

clean :
	rm -f *.o
	rm -f max7219matrix
	cd $(ROOT)/lib-device && make -f Makefile.Linux clean
	
$(ROOT)/lib-device/lib_linux/libdevice.a :
	cd $(ROOT)/lib-device && make -f Makefile.Linux 'DEFINES=-DRASPPI -DNDEBUG'

$(ROOT)/lib-i2c/lib_linux/libi2c.a :
	cd $(ROOT)/lib-i2c && make -f Makefile.Linux 'DEFINES=-DRASPPI -DNDEBUG'
	
max7219matrix : Makefile max7219matrix.cpp $(ROOT)/lib-device/lib_linux/libdevice.a
	$(CPP) max7219matrix.cpp $(INCLUDES) $(COPS) -o max7219matrix $(LIB) $(LDLIBS) 
	
