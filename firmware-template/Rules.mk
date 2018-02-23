PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

LIBS += hal uuid esp8266 display properties c++ utils fb ff11 emmc bob i2c bcm2835

DEFINES := $(addprefix -D,$(DEFINES))

# The variable for the firmware include directories
INCDIRS = ../include $(wildcard ./include) $(wildcard ./*/include)
INCDIRS := $(addprefix -I,$(INCDIRS))

# The variable for the libraries include directory
LIBINCDIRS = $(addprefix -I../lib-,$(LIBS))
LIBINCDIRS := $(addsuffix /include, $(LIBINCDIRS))

# The variables for the ld -L flag
LIB6  = $(addprefix -L../lib-,$(LIBS))
LIB6 := $(addsuffix /lib, $(LIB6))
LIB7  = $(addprefix -L../lib-,$(LIBS))
LIB7 := $(addsuffix /lib7, $(LIB7))

# The variable for the ld -l flag 
LDLIBS := $(addprefix -l,$(LIBS))

# The variables for the dependency check 
LIBDEP = $(addprefix ../lib-,$(LIBS))
LIB6DEP = $(addsuffix /lib/lib, $(LIBDEP))
LIB6DEP := $(join $(LIB6DEP), $(LIBS))
LIB6DEP := $(addsuffix .a, $(LIB6DEP))
LIB7DEP = $(addsuffix /lib7/lib, $(LIBDEP))
LIB7DEP := $(join $(LIB7DEP), $(LIBS))
LIB7DEP := $(addsuffix .a, $(LIB7DEP))

COPS_COMMON = -DBARE_METAL $(DEFINES) #-DNDEBUG
COPS_COMMON += $(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -nostdinc -nostdlib -ffreestanding -mhard-float -mfloat-abi=hard #-fstack-usage

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7 #-fopt-info-vec-optimized
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

SOURCE = ./

BUILD = build/
BUILD7 = build7/

C_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
C_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS7 += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

BUILD_DIRS := $(addprefix build/,$(SRCDIR))
BUILD7_DIRS := $(addprefix build7/,$(SRCDIR))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)
OBJECTS7 := $(ASM_OBJECTS7) $(C_OBJECTS7)

TARGET = kernel.img
TARGET7 = kernel7.img

LIST = kernel.list
LIST7 = kernel7.list

MAP = kernel.map
MAP7 = kernel7.map

LINKER = firmware/memmap

define compile-objects6
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti -std=c++11 $(COPS) -c $$< -o $$@	
endef

define compile-objects7
$(BUILD7)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS7) -c $$< -o $$@
	
$(BUILD7)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti -std=c++11 $(COPS7) -c $$< -o $$@		
endef

THISDIR = $(CURDIR)

all : builddirs prerequisites $(TARGET) $(TARGET7)
	
.PHONY: clean builddirs

buildlibs:
	cd .. && ./makeall-lib.sh && cd $(THISDIR)

builddirs:
	@mkdir -p $(BUILD_DIRS) $(BUILD7_DIRS)

clean:
	rm -rf $(BUILD) $(BUILD7)
	rm -f $(TARGET) $(TARGET7)
	rm -f $(MAP) $(MAP7)
	rm -f $(LIST) $(LIST7)

# Build kernel.img

$(BUILD)vectors.o : firmware/vectors.S
	$(AS) $(COPS) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD)vectors.o
	
$(BUILD)main.elf : Makefile $(LINKER) $(BUILD)vectors.o $(OBJECTS) $(LIB6DEP)
	$(LD) $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIB6) $(LDLIBS)
	$(PREFIX)objdump -D $(BUILD)main.elf | $(PREFIX)c++filt > $(LIST)

$(TARGET) : $(BUILD)main.elf
	$(PREFIX)objcopy $(BUILD)main.elf -O binary $(TARGET)

# Build kernel7.img

$(BUILD7)vectors.o : firmware/vectors.S
	$(AS) $(COPS7) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD7)vectors.o
	
$(BUILD7)main.elf : Makefile $(LINKER) $(BUILD7)vectors.o $(OBJECTS7) $(LIB7DEP)
	$(LD) $(BUILD7)vectors.o $(OBJECTS7) -Map $(MAP7) -T $(LINKER) -o $(BUILD7)main.elf $(LIB7) $(LDLIBS)
	$(PREFIX)objdump -D $(BUILD7)main.elf | $(PREFIX)c++filt > $(LIST7)

$(TARGET7) : $(BUILD7)main.elf
	$(PREFIX)objcopy $(BUILD7)main.elf -O binary $(TARGET7)
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects6,$(bdir))))
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects7,$(bdir))))
