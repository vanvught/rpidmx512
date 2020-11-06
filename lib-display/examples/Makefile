PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-display/lib_linux
LDLIBS := -ldisplay
LIBDEP := $(ROOT)/lib-display/lib_linux/libdisplay.a

INCLUDES := -I$(ROOT)/lib-display/include -I$(ROOT)/lib-hal/include

COPS := -Wall -Werror -O2 -fno-rtti -std=c++11 -DRASPPI -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : detect oled128x32 ssd1306 ssd1311 lcd2004 lcd1602 

clean :
	rm -f *.o
	rm -f detect oled128x32 ssd1306 ssd1311
	cd $(ROOT)/lib-display && make -f Makefile.Linux clean

mircos.o	: Makefile micros.c
	$(CC) -c micros.c
	
$(ROOT)/lib-display/lib_linux/libdisplay.a :
	cd $(ROOT)/lib-display && make -f Makefile.Linux 'DEFINES=-DRASPPI -DENABLE_SSD1311 -DENABLE_TC1602 -DENABLE_LCDBW -DENABLE_CURSOR_MODE -DNO_HAL -DNDEBUG'

$(ROOT)/lib-i2c/lib_linux/libi2c.a :
	cd $(ROOT)/lib-i2c && make -f Makefile.Linux 'DEFINES=-DRASPPI -DNDEBUG'
	
detect : Makefile detect.cpp $(ROOT)/lib-display/lib_linux/libdisplay.a micros.o
	$(CPP) detect.cpp micros.o $(INCLUDES) $(COPS) -o detect $(LIB) $(LDLIBS) 
	
oled128x32 : Makefile detect.cpp $(ROOT)/lib-display/lib_linux/libdisplay.a
	$(CPP) oled128x32.cpp $(INCLUDES) $(COPS) -o oled128x32 $(LIB) $(LDLIBS) 
	
ssd1306 : Makefile ssd1306.cpp $(ROOT)/lib-display/lib_linux/libdisplay.a 
	$(CPP) ssd1306.cpp micros.o $(INCLUDES) $(COPS) -o ssd1306 $(LIB) $(LDLIBS)

ssd1311 : Makefile ssd1311.cpp $(ROOT)/lib-display/lib_linux/libdisplay.a 
	$(CPP) ssd1311.cpp micros.o $(INCLUDES) $(COPS) -o ssd1311 $(LIB) $(LDLIBS)

lcd1602 : Makefile lcd1602.cpp $(ROOT)/lib-display/lib_linux/libdisplay.a micros.o
	$(CPP) lcd1602.cpp micros.o $(INCLUDES) $(COPS) -o lcd1602 $(LIB) $(LDLIBS)
	
lcd2004 : Makefile lcd2004.cpp $(ROOT)/lib-display/lib_linux/libdisplay.a micros.o
	$(CPP) lcd2004.cpp micros.o $(INCLUDES) $(COPS) -o lcd2004 $(LIB) $(LDLIBS)
