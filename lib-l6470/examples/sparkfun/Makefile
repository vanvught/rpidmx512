PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../../..

LIB := -L$(ROOT)/lib-l6470/lib_linux
LDLIBS := -ll6470 
LIBDEP := $(ROOT)/lib-l6470/lib_linux/libl6470.a

INCLUDES := -I$(ROOT)/lib-l6470/include

COPS := -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : SparkFunGetSetParamTest simple_move

clean :
	rm -f *.o
	rm -f *.lst
	rm -f SparkFunGetSetParamTest simple_move
	cd $(ROOT)/lib-l6470 && make -f Makefile.Linux clean
	
$(ROOT)/lib-l6470/lib_linux/libl6470.a :
	cd $(ROOT)/lib-l6470 && make -f Makefile.Linux "DEF=-DRASPPI"
	
SparkFunGetSetParamTest : Makefile SparkFunGetSetParamTest.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) SparkFunGetSetParamTest.cpp $(INCLUDES) $(COPS) -o SparkFunGetSetParamTest $(LIB) $(LDLIBS)
	
simple_move : Makefile simple_move.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) simple_move.cpp $(INCLUDES) $(COPS) -o simple_move $(LIB) $(LDLIBS)	
