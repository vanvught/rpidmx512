PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..
LIB = -L$(ROOT)/lib-wiznet/lib_linux
LIBDEP = $(ROOT)/lib-wiznet/lib_linux/libwiznet.a
INCLUDES = -I$(ROOT)/lib-wiznet/include -I$(ROOT)/lib-wiznet/ioLibrary_Driver/Ethernet
COPS = -Wall -Werror -O2  #-DNDEBUG
BCM2835 = $(ROOT)/lib-bcm2835_raspbian

ifneq "$(wildcard $(BCM2835) )" ""
	LIB += -L$(BCM2835)/lib_linux
	LDLIBS = -lbcm2835_raspbian
	INCLUDES += -I$(BCM2835)/include
else
	LDLIBS = -lbcm2835
endif

all : w5100_static_ip w5500_static_ip

clean :
	rm -f *.o
	rm -f *.lst
	rm -f w5100_static_ip w5500_static_ip
	cd $(ROOT)/lib-wiznet && make -f Makefile.Linux "DEF=-DRASPPI" clean
	
$(ROOT)/lib-wiznet/lib_linux/libwiznet5100.a :
	cd $(ROOT)/lib-wiznet && make -f Makefile.Linux "DEF=-DRASPPI"

$(ROOT)/lib-wiznet/lib_linux/libwiznet5500.a :
	cd $(ROOT)/lib-wiznet && make -f Makefile.Linux "DEF=-DRASPPI"
	
w5100_static_ip : Makefile w5x00_static_ip.c $(ROOT)/lib-wiznet/lib_linux/libwiznet5100.a
	$(CC) w5x00_static_ip.c $(INCLUDES) $(COPS) -D_WIZCHIP_=5100 -o w5100_static_ip $(LIB) -lwiznet5100 $(LDLIBS)
	$(PREFIX)objdump -D w5100_static_ip | $(PREFIX)c++filt > w5100_static_ip.lst

w5500_static_ip : Makefile w5x00_static_ip.c $(ROOT)/lib-wiznet/lib_linux/libwiznet5500.a
	$(CC) w5x00_static_ip.c $(INCLUDES) $(COPS) -D_WIZCHIP_=5500 -o w5500_static_ip $(LIB) -lwiznet5500 $(LDLIBS)
	$(PREFIX)objdump -D w5500_static_ip | $(PREFIX)c++filt > w5500_static_ip.lst
	
