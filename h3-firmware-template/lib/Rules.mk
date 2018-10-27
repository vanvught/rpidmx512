PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

PLATFORM?=ORANGE_PI
CONSOLE?=

SRCDIR = src src/h3 $(EXTRA_SRCDIR)

INCLUDES:= -I./include -I../include -I../lib-debug/include -I../lib-h3/include -I../lib-arm/include 
INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))

DEFINES:=$(addprefix -D,$(DEFINES)) -D$(PLATFORM)

ifneq ($(CONSOLE),)
	DEFINES+=-D$(CONSOLE)
endif

COPS=-DBARE_METAL -DH3 $(DEFINES) $(INCLUDES)
COPS+=-mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7 -mhard-float -mfloat-abi=hard
COPS+=-Wall -Werror -O2 -nostartfiles -ffreestanding -nostdinc -nostdlib -fno-exceptions -fno-unwind-tables #-fstack-usage

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME:=$(patsubst lib-%,%,$(CURR_DIR))

BUILD = build_h3/
BUILD_DIRS:=$(addprefix build_h3/,$(SRCDIR))

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

BUILD_DIRS:= $(addprefix build_h3/,$(SRCDIR))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS)

TARGET = lib_h3/lib$(LIB_NAME).a 

LIST = lib.list

define compile-objects
$(BUILD)$1/%.o: $1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.cpp
	$(CPP) $(COPS) -fno-rtti -std=c++11 -nostdinc++ -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.S
	$(CC) $(COPS) -D__ASSEMBLY__ -c $$< -o $$@	
endef

all : builddirs $(TARGET)

.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD_DIRS)
	mkdir -p lib_h3

clean :
	rm -rf build_h3
	rm -rf lib_h3
	
# Build lib

$(TARGET): Makefile.H3 $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -D $(TARGET) | $(PREFIX)c++filt > lib_h3/$(LIST)
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
	