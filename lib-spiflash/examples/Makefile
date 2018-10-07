PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-spiflash/lib_linux
LDLIBS := -lspiflash
LIBDEP := $(ROOT)/lib-spiflash/lib_linux/libspiflash.a

INCLUDES := -I$(ROOT)/lib-spiflash/include

COPS := -Wall -Werror -O2 -DRASPI #-DNDEBUG

BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS += -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS += -lbcm2835
endif

all : detect

clean :
	rm -f *.o
	rm -f detect 
	cd $(ROOT)/lib-spiflash && make -f Makefile.Linux clean
	
$(ROOT)/lib-spiflash/lib_linux/libspiflash.a :
	cd $(ROOT)/lib-spiflash && make -f Makefile.Linux	

detect : Makefile detect.c $(ROOT)/lib-spiflash/lib_linux/libspiflash.a
	$(CC) detect.c $(INCLUDES) $(COPS) -o detect $(LIB) $(LDLIBS)
	

