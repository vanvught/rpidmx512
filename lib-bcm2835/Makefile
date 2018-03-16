PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

SRCDIR = src arm arm/aeabi device

COPS_COMMON = -DBARE_METAL -DNDEBUG
COPS_COMMON += -I ./include -I ./../include
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -nostdinc -nostdlib -ffreestanding -mhard-float -mfloat-abi=hard

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

TARGET = lib/libbcm2835.a 
TARGET7 = lib7/libbcm2835.a

BUILD = build/
BUILD7 = build7/

BUILD_DIRS := $(addprefix build/,$(SRCDIR))
BUILD7_DIRS := $(addprefix build7/,$(SRCDIR))

C_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
ASM_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))
C_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
ASM_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

BUILD_DIRS := $(addprefix build/,$(SRCDIR))
BUILD7_DIRS := $(addprefix build7/,$(SRCDIR))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)
OBJECTS7 := $(ASM_OBJECTS7) $(C_OBJECTS7)

LIST = lib.list
LIST7 = lib7.list

define compile-objects6
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.S
	$(CC) $(COPS) -D__ASSEMBLY__ -c $$< -o $$@	
endef

define compile-objects7
$(BUILD7)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS7) -c $$< -o $$@
	
$(BUILD7)$1/%.o: $(SOURCE)$1/%.S	
	$(CC) $(COPS7) -D__ASSEMBLY__ -c $$< -o $$@		
endef

all : builddirs $(TARGET) $(TARGET7)

.PHONY: clean builddirs

builddirs:
	@mkdir -p $(BUILD_DIRS) $(BUILD7_DIRS)

clean :
	rm -rf $(BUILD) $(BUILD7)
	rm -f $(TARGET) $(TARGET7)	
	rm -f $(LIST) $(LIST7)	

# Build lib

$(TARGET): Makefile $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS) 
	$(PREFIX)objdump -D $(TARGET) > $(LIST)

# Build lib7

$(TARGET7): Makefile $(OBJECTS7)
	$(AR) -r $(TARGET7) $(OBJECTS7) 
	$(PREFIX)objdump -D $(TARGET7) > $(LIST7)
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects6,$(bdir))))
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects7,$(bdir))))