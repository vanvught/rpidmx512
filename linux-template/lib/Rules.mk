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

INCLUDES:=-I./include -I../lib-hal/include -I../lib-debug/include
INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))

detected_OS := $(shell uname 2>/dev/null || echo Unknown)
detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))

$(info $$detected_OS [${detected_OS}])

ifeq ($(detected_OS),Darwin) 
endif

ifeq ($(detected_OS),Linux) 
	ifneq (, $(shell which /opt/vc/bin/vcgencmd))
	
		BCM2835 = ./../lib-bcm2835_raspbian
	
		ifneq "$(wildcard $(BCM2835) )" ""
			INCLUDES+=-I../lib-bcm2835_raspbian/include
		endif
	
		ifneq ($(findstring RASPPI,$(DEFINES)),RASPPI)
			DEFINES+=-DRASPPI
		endif
	endif
endif

$(info $$DEFINES [${DEFINES}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
 
COPS=$(DEFINES) $(MAKE_FLAGS) $(INCLUDES)
COPS+=-O2 -Wall -Werror -Wextra -Wpedantic -Wunused -Wsign-conversion #-Wconversion

CCPOPS=-fno-rtti -fno-exceptions -fno-unwind-tables -Wnon-virtual-dtor
ifeq ($(detected_OS),Darwin) 
else
CCPOPS+=-Wuseless-cast -Wold-style-cast
endif

ifeq ($(detected_OS),Cygwin)
	CCPOPS+=-std=gnu++11
else
	CCPOPS+=-std=c++11
endif	

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
