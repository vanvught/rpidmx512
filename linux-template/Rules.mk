PREFIX ?=
DEF ?= 

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

DEFINES:=$(addprefix -D,$(DEFINES))

LIBS += network

ifeq ($(findstring WIZNET,$(DEFINES)),WIZNET)
	LIBS += wiznet
endif

LIBS += hal properties

ifneq (, $(shell which /opt/vc/bin/vcgencmd))
	LIBS += bob i2c
	BCM2835 = ./../lib-bcm2835_raspbian
	ifneq "$(wildcard $(BCM2835) )" ""
		LIBS += bcm2835_raspbian
	else
		LIBS += bcm2835
	endif
	DEFINES+=-DRASPPI
	ifeq ($(findstring ENABLE_SPIFLASH,$(DEFINES)),ENABLE_SPIFLASH)
		LIBS:=spiflashstore spiflash $(LIBS)
	endif
endif

# The variable for the firmware include directories
INCDIRS=$(wildcard ./include) $(wildcard ./*/include)
INCDIRS:=$(addprefix -I,$(INCDIRS))

# The variable for the libraries include directory
LIBINCDIRS=$(addprefix -I../lib-,$(LIBS))
LIBINCDIRS:=$(addsuffix /include, $(LIBINCDIRS))

# The variables for the ld -L flag
LIB=$(addprefix -L../lib-,$(LIBS))
LIB:=$(addsuffix /lib_linux, $(LIB))

# The variable for the ld -l flag 
LDLIBS:=$(addprefix -l,$(LIBS))

# The variables for the dependency check 
LIBDEP = $(addprefix ../lib-,$(LIBS))
LIBDEP:=$(addsuffix /lib_linux/lib, $(LIBDEP))
LIBDEP:=$(join $(LIBDEP), $(LIBS))
LIBDEP:=$(addsuffix .a, $(LIBDEP))

COPS=$(DEFINES) #-DNDEBUG
COPS+=$(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS+=-Wall -Werror -O2

SOURCE = ./

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))

BUILD = build_linux/

C_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

BUILD_DIRS:=$(addprefix build_linux/,$(SRCDIR))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS)

TARGET = $(CURR_DIR)

define compile-objects
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) $(COPS) -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti -std=c++11 -c $$< -o $$@	
endef

THISDIR = $(CURDIR)

all : builddirs prerequisites $(TARGET)
	
.PHONY: clean builddirs

buildlibs:
	cd .. && ./makeall_linux-lib.sh && cd $(THISDIR)

builddirs:
	@mkdir -p $(BUILD_DIRS)

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)

$(CURR_DIR) : Makefile $(LINKER) $(OBJECTS) $(LIBDEP)
	$(CPP) $(OBJECTS) -o $(CURR_DIR) $(LIB) $(LDLIBS) -luuid
	$(PREFIX)objdump -D $(TARGET) | $(PREFIX)c++filt > linux.lst

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
