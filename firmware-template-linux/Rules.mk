PREFIX ?=
DEF ?= 

CC	= $(PREFIX)gcc
CPP =$(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

$(info [${CURDIR}])

detected_OS := $(shell uname 2>/dev/null || echo Unknown)
detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))

$(info $$detected_OS [${detected_OS}])

ifeq ($(detected_OS),Darwin)
	LINUX=1
endif

ifeq ($(detected_OS),Linux) 
	LINUX=1
endif

include ../firmware-template/libs.mk

TTT=uuid
TMPVAR:=$(LIBS)
LIBS=$(filter-out $(TTT), $(TMPVAR))
LDLIBS=

DEFINES:=$(addprefix -D,$(DEFINES))
DEFINES+=-DDISABLE_TFTP  
DEFINES+=-DENABLE_HTTPD
DEFINES+=-DCONFIG_STORE_USE_FILE 
DEFINES+=-DCONFIG_MDNS_DOMAIN_REVERSE
DEFINES+=-DISABLE_INTERNAL_RTC
DEFINES+=$(addprefix -I,$(EXTRA_INCLUDES))

ifeq ($(findstring ARTNET_VERSION=4,$(DEFINES)),ARTNET_VERSION=4)
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(DEFINES)),ARTNET_HAVE_DMXIN)
		DEFINES+=-DE131_HAVE_DMXIN
	endif
endif

ifneq ($(findstring _TIME_STAMP_YEAR_,$(DEFINES)), _TIME_STAMP_YEAR_)
	DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")
endif

# The variable for the firmware include directories
INCDIRS=$(wildcard ./lib) $(wildcard ./include) $(wildcard ./*/include) ../firmware-template-linux/include
INCDIRS:=$(addprefix -I,$(INCDIRS)) -I../lib-display/include

# The variable for the libraries include directory
LIBINCDIRS=$(addprefix -I../lib-,$(DEFAULT_INCLUDES))
LIBINCDIRS+=$(addprefix -I../lib-,$(LIBS))
ifeq ($(findstring CONFIG_DISPLAY_USE_CUSTOM,$(DEFINES)),CONFIG_DISPLAY_USE_CUSTOM)
	ifneq ($(CONFIG_DISPLAY_LIB),)
		LIBINCDIRS+=$(addprefix -I../lib-,$(CONFIG_DISPLAY_LIB))
	endif
endif
LIBINCDIRS:=$(addsuffix /include, $(LIBINCDIRS))

DEFINES+=$(INCDIRS) $(LIBINCDIRS)

$(info $$LIBS [${LIBS}])

# The variables for the ld -L flag
LIB=$(addprefix -L../lib-,$(LIBS))
LIB:=$(addsuffix /lib_linux, $(LIB))

# The variable for the ld -l flag 
LDLIBS+=$(addprefix -l,$(LIBS))

ifdef LINUX
	ifneq (, $(shell which vcgencmd))
		BCM2835 = ./../lib-bcm2835_raspbian
		ifneq "$(wildcard $(BCM2835) )" ""
			LIB+=-L../lib-bcm2835_raspbian/lib_linux
			LDLIBS+=-lbcm2835_raspbian
			DEFINES+=-I../lib-bcm2835_raspbian/include
		else
			LDLIBS+=-lbcm2835
		endif
		DEFINES+=-DRASPPI
		DEFINES+=-DBCM2835_NO_DELAY_COMPATIBILITY
	endif
endif

$(info $$LDLIBS [${LDLIBS}])

# The variables for the dependency check 
LIBDEP=$(addprefix ../lib-,$(LIBS))

COPS=$(DEFINES)
COPS+=-g -Wall -Werror -Wextra -pedantic 
COPS+=-Wunused #-Wsign-conversion #-Wconversion
COPS+=-fstack-protector-all

CCPOPS=-fno-rtti -fno-exceptions -fno-unwind-tables -Wnon-virtual-dtor

ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
else
	COPS+=-Wduplicated-cond -Wlogical-op #-Wduplicated-branches
#	CCPOPS+=-Wuseless-cast -Wold-style-cast
endif
CCPOPS+=-std=c++20

COPS+=-ffunction-sections -fdata-sections

SOURCE = ./

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))

BUILD = build_linux/

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS+=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

BUILD_DIRS:=$(addprefix build_linux/,$(SRCDIR))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS)

TARGET = $(CURR_DIR)

define compile-objects
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) $(COPS) $(CCPOPS) -c $$< -o $$@	
endef

THISDIR = $(CURDIR)

all : builddirs prerequisites $(TARGET)
	
.PHONY: clean builddirs

builddirs:
	@mkdir -p $(BUILD_DIRS)
	
clean: $(LIBDEP)
	rm -rf $(BUILD)
	rm -f $(TARGET)
	rm -f include/sofware_version_id.h
	
#
# Libraries
#

.PHONY: libdep $(LIBDEP)

lisdep: $(LIBDEP)

$(LIBDEP):
	$(MAKE) -f Makefile.Linux $(MAKECMDGOALS) 'MAKE_FLAGS=$(DEFINES)' -C $@ 

# Build uImage

$(BUILD_DIRS) :
	mkdir -p $(BUILD_DIRS)
		
$(CURR_DIR) : Makefile $(LINKER) $(OBJECTS) $(LIBDEP)
	$(info $$TARGET [${TARGET}])
	$(CPP) $(OBJECTS) -o $(CURR_DIR) $(LIB) $(LDLIBS) -luuid -lpthread -lz
	$(PREFIX)objdump -d $(TARGET) | $(PREFIX)c++filt > linux.lst

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
