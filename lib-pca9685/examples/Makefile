PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-pca9685/lib_linux
LDLIBS := -lpca9685 
LIBDEP := $(ROOT)/lib-pca9685/lib_linux/libpca9685.a

INCLUDES := -I$(ROOT)/lib-pca9685/include

COPS := -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : simple pwmled servo

clean :
	rm -f *.o
	rm -f *.lst
	rm -f simple pwmled servo
	cd $(ROOT)/lib-pca9685 && make -f Makefile.Linux clean
	
$(ROOT)/lib-pca9685/lib_linux/libpca9685.a :
	cd $(ROOT)/lib-pca9685 && make -f Makefile.Linux "DEF=-DRASPPI"
	
simple : Makefile simple.cpp $(ROOT)/lib-pca9685/lib_linux/libpca9685.a
	$(CPP) simple.cpp $(INCLUDES) $(COPS) -o simple $(LIB) $(LDLIBS)
	$(PREFIX)objdump -D simple | $(PREFIX)c++filt > simple.lst

pwmled : Makefile pwmled.cpp $(ROOT)/lib-pca9685/lib_linux/libpca9685.a
	$(CPP) pwmled.cpp $(INCLUDES) $(COPS) -o pwmled $(LIB) $(LDLIBS)
	$(PREFIX)objdump -D pwmled | $(PREFIX)c++filt > pwmled.lst

servo : Makefile servo.cpp $(ROOT)/lib-pca9685/lib_linux/libpca9685.a
	$(CPP) servo.cpp $(INCLUDES) $(COPS) -o servo $(LIB) $(LDLIBS)
	$(PREFIX)objdump -D servo | $(PREFIX)c++filt > servo.lst
	