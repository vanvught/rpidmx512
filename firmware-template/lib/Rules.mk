PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

SRCDIR = src src/rpi $(EXTRA_SRCDIR)

INCLUDES := -I./include -I../include -I../lib-bcm2835/include -I../lib-arm/include -I../lib-debug/include 
INCLUDES += $(addprefix -I,$(EXTRA_INCLUDES))

DEFINES := $(addprefix -D,$(DEFINES))

COPS_COMMON = -DBARE_METAL -DHAVE_I2C -DHAVE_SPI $(DEFINES) $(INCLUDES)
COPS_COMMON += -Wall -Werror -O2 -nostartfiles -ffreestanding -nostdinc -nostdlib -mhard-float -mfloat-abi=hard -fno-exceptions -fno-unwind-tables #-fstack-usage

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

CURR_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME := $(patsubst lib-%,%,$(CURR_DIR))

BUILD = build/
BUILD7 = build7/

BUILD_DIRS := $(addprefix build/,$(SRCDIR))
BUILD7_DIRS := $(addprefix build7/,$(SRCDIR))

C_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

C_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS)
OBJECTS7 := $(ASM_OBJECTS7) $(C_OBJECTS7) $(CPP_OBJECTS7)

TARGET = lib/lib$(LIB_NAME).a 
TARGET7 = lib7/lib$(LIB_NAME).a

LIST = lib.list
LIST7 = lib7.list

define compile-objects6
$(BUILD)$1/%.o: $1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.cpp
	$(CPP) $(COPS) -fno-rtti -std=c++11 -nostdinc++ -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.S
	$(CC) $(COPS) -D__ASSEMBLY__ -c $$< -o $$@	
endef

define compile-objects7
$(BUILD7)$1/%.o: $1/%.c
	$(CC) $(COPS7) -c $$< -o $$@
	
$(BUILD7)$1/%.o: $1/%.cpp
	$(CPP) $(COPS7) -fno-rtti -std=c++11 -nostdinc++ -c $$< -o $$@	
	
$(BUILD7)$1/%.o: $1/%.S	
	$(CC) $(COPS7) -D__ASSEMBLY__ -c $$< -o $$@		
endef

all : builddirs $(TARGET) $(TARGET7)

.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD_DIRS) $(BUILD7_DIRS)
	mkdir -p lib lib7

clean :
	rm -rf $(BUILD) $(BUILD7)
	rm -rf lib/ lib7/	

# ARM v6
	
$(TARGET): Makefile $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -D $(TARGET) | $(PREFIX)c++filt > lib/$(LIST)
	
# ARM v7		

$(TARGET7): Makefile $(OBJECTS7)
	$(AR) -r $(TARGET7) $(OBJECTS7)
	$(PREFIX)objdump -D $(TARGET7) | $(PREFIX)c++filt > lib7/$(LIST7)
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects6,$(bdir))))
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects7,$(bdir))))	
