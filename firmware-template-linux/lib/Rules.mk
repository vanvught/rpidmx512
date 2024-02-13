PREFIX ?=

CC=$(PREFIX)gcc
CPP=$(PREFIX)g++
AS=$(CC)
LD=$(PREFIX)ld
AR=$(PREFIX)ar

$(info [${CURDIR}])

CURR_DIR :=$(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME :=$(patsubst lib-%,%,$(CURR_DIR))

DEFINES:=$(addprefix -D,$(DEFINES))
DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")
DEFINES+=-DDISABLE_TFTP  
DEFINES+=-DENABLE_HTTPD
DEFINES+=-DCONFIG_STORE_USE_FILE 
DEFINES+=-DCONFIG_MDNS_DOMAIN_REVERSE
DEFINES+=-DDISABLE_INTERNAL_RTC

ifeq ($(findstring ARTNET_VERSION=4,$(DEFINES)),ARTNET_VERSION=4)
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(DEFINES)),ARTNET_HAVE_DMXIN)
		DEFINES+=-DE131_HAVE_DMXIN
	endif
endif

INCLUDES:=-I./include -I../lib-configstore/include -I../lib-hal/include -I../lib-display/include -I../lib-debug/include
INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))
ifeq ($(findstring CONFIG_DISPLAY_USE_CUSTOM,$(DEFINES)),CONFIG_DISPLAY_USE_CUSTOM)
	ifneq ($(CONFIG_DISPLAY_LIB),)
		INCLUDES+=-I../lib-$(CONFIG_DISPLAY_LIB)/include
	endif
endif

detected_OS := $(shell uname 2>/dev/null || echo Unknown)
detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))

$(info $$detected_OS [${detected_OS}])

ifeq ($(detected_OS),Darwin) 
endif

ifeq ($(detected_OS),Linux) 
	ifneq (, $(shell which vcgencmd))
		BCM2835 = ./../lib-bcm2835_raspbian
		ifneq "$(wildcard $(BCM2835) )" ""
			INCLUDES+=-I../lib-bcm2835_raspbian/include
		endif
		ifneq ($(findstring RASPPI,$(DEFINES)),RASPPI)
			DEFINES+=-DRASPPI
		endif
		DEFINES+=-DBCM2835_NO_DELAY_COMPATIBILITY
	endif
endif

$(info $$DEFINES [${DEFINES}])
$(info $$INCLUDES [${INCLUDES}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
 
COPS=$(DEFINES) $(MAKE_FLAGS) $(INCLUDES)
COPS+=-g -Wall -Werror -Wextra -Wpedantic 
COPS+=-Wunused #-Wsign-conversion -Wconversion
COPS+=-fstack-protector-all

ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
else
	COPS+=-Wduplicated-cond -Wlogical-op #-Wduplicated-branches
	CCPOPS+=-Wuseless-cast -Wold-style-cast
endif

CCPOPS=-fno-rtti -fno-exceptions -fno-unwind-tables -Wnon-virtual-dtor
CCPOPS+=-std=c++20

COPS+=-ffunction-sections -fdata-sections

SRCDIR = src src/linux $(EXTRA_SRCDIR)

BUILD = build_linux/
BUILD_DIRS:=$(addprefix $(BUILD),$(SRCDIR))
$(info $$BUILD_DIRS [${BUILD_DIRS}])

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS)
$(info $$OBJECTS [${OBJECTS}])

TARGET = lib_linux/lib$(LIB_NAME).a 

LIST = lib.list

define compile-objects
$(BUILD)$1/%.o: $1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.cpp
	$(CPP) $(COPS) $(CCPOPS) -c $$< -o $$@
endef

all : builddirs $(TARGET)

.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD_DIRS)
	mkdir -p lib_linux

clean:
	rm -rf $(BUILD)
	rm -rf lib_linux	
	
$(BUILD_DIRS) :	
	mkdir -p $(BUILD_DIRS)
	mkdir -p lib_linux
	
$(TARGET): Makefile.Linux $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -d $(TARGET) | $(PREFIX)c++filt > lib_linux/$(LIST)
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
