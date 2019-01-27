PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-bob/lib_linux -L$(ROOT)/lib-i2c/lib_linux
LDLIBS := -lbob -li2c
LIBDEP := $(ROOT)/lib-bob/lib_linux/libbob.a $(ROOT)/lib-i2c/lib_linux/libi2c.a

INCLUDES := -I$(ROOT)/lib-bob/include -I$(ROOT)/lib-i2c/include

COPS := -Wall -Werror -O2 -DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq ("$(wildcard $(BCM2835))", "")
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : mcp23017 si5351a

clean :
	rm -f *.o
	rm -f *.lst
	rm -f mcp23017
	cd $(ROOT)/lib-bob && make -f Makefile.Linux clean
	cd $(ROOT)/lib-i2c && make -f Makefile.Linux clean
	
$(ROOT)/lib-bob/lib_linux/libbob.a :
	cd $(ROOT)/lib-bob && make -f Makefile.Linux
	
$(ROOT)/lib-i2c/lib_linux/libi2c.a :
	cd $(ROOT)/lib-i2c && make -f Makefile.Linux
	
mcp23017 : Makefile mcp23017.c $(LIBDEP)
	$(CC) mcp23017.c $(INCLUDES) $(COPS) -o mcp23017 $(LIB) $(LDLIBS)
	
si5351a : Makefile si5351a.c $(LIBDEP)
	$(CC) si5351a.c $(INCLUDES) $(COPS) -o si5351a $(LIB) $(LDLIBS)	
