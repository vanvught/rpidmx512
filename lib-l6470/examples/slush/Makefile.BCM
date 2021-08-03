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

all : input_output limit_switch precise_move simple_move get_set_param_test get_temp

clean :
	rm -f *.o
	rm -f *.lst
	rm -f input_output limit_switch precise_move simple_move get_set_param_test get_temp
	cd $(ROOT)/lib-l6470 && make -f Makefile.Linux clean
	
$(ROOT)/lib-l6470/lib_linux/libl6470.a :
	cd $(ROOT)/lib-l6470 && make -f Makefile.Linux "DEF=-DRASPPI"
	
input_output : Makefile input_output.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) input_output.cpp $(INCLUDES) $(COPS) -o input_output $(LIB) $(LDLIBS)

limit_switch : Makefile limit_switch.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) limit_switch.cpp $(INCLUDES) $(COPS) -o limit_switch $(LIB) $(LDLIBS)
	
precise_move : Makefile precise_move.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) precise_move.cpp $(INCLUDES) $(COPS) -o precise_move $(LIB) $(LDLIBS)	

simple_move : Makefile simple_move.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) simple_move.cpp $(INCLUDES) $(COPS) -o simple_move $(LIB) $(LDLIBS)
	
get_set_param_test : Makefile get_set_param_test.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) get_set_param_test.cpp $(INCLUDES) $(COPS) -o get_set_param_test $(LIB) $(LDLIBS)
	
get_temp : Makefile get_temp.cpp $(ROOT)/lib-l6470/lib_linux/libl6470.a
	$(CPP) get_temp.cpp $(INCLUDES) $(COPS) -o get_temp $(LIB) $(LDLIBS) -lm
	