PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT = ./../..

LIB := -L$(ROOT)/lib-device/lib_linux -L$(ROOT)/lib-linux/lib_linux
LDLIBS := -ldevice -llinux
LIBDEP := $(ROOT)/lib-device $(ROOT)/lib-linux
INCLUDES := -I$(ROOT)/lib-device/include -I$(ROOT)/lib-linux/include
COPS := -Wall -Werror -O2 -fno-rtti -std=c++20 -DNDEBUG
DEFINES := -DDISABLE_RTC -I./../lib-superloop/include

all : max7219matrix mcp3424

clean :
	rm -f *.o
	rm -f max7219matrix
	cd $(ROOT)/lib-device && make -f Makefile.Linux clean
	cd $(ROOT)/lib-linux && make -f Makefile.Linux clean
	
.PHONY: libdep $(LIBDEP)

lisdep: $(LIBDEP)
	
$(LIBDEP):
	$(MAKE) -f Makefile.Linux 'MAKE_FLAGS=$(DEFINES)' -C $@ 
	
max7219matrix : Makefile max7219matrix.cpp $(LIBDEP)
	$(CPP) max7219matrix.cpp $(INCLUDES) $(COPS) -o max7219matrix $(LIB) $(LDLIBS) 
	
mcp3424 : Makefile mcp3424.cpp $(LIBDEP)
	$(CPP) mcp3424.cpp $(INCLUDES) $(COPS) -o mcp3424 $(LIB) $(LDLIBS) 
