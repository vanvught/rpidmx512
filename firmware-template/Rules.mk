ARMGNU ?= arm-none-eabi

LIBS += hal uuid ff11 emmc fb lcd bob i2c utils bcm2835

DEFINES := $(addprefix -D,$(DEFINES))

# The variable for the firmware include directories
INCDIRS = $(wildcard ./include) $(wildcard ./*/include)
INCDIRS := $(addprefix -I,$(INCDIRS))

# The variable for the libraries include directory
LIBINCDIRS = $(addprefix -I../lib-,$(LIBS))
LIBINCDIRS := $(addsuffix /include, $(LIBINCDIRS))

# The variables for the ld -L flag
LIB6  = $(addprefix -L../lib-,$(LIBS))
LIB6 := $(addsuffix /lib, $(LIB6))
LIB7  = $(addprefix -L../lib-,$(LIBS))
LIB7 := $(addsuffix /lib7, $(LIB7))
LIB8  = $(addprefix -L../lib-,$(LIBS))
LIB8 := $(addsuffix /lib8, $(LIB8))

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
LIB8DEP = $(addsuffix /lib8/lib, $(LIBDEP))
LIB8DEP := $(join $(LIB8DEP), $(LIBS))
LIB8DEP := $(addsuffix .a, $(LIB8DEP))

COPS_COMMON = -DBARE_METAL $(DEFINES) #-DNDEBUG
COPS_COMMON += $(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -ffreestanding -mhard-float -mfloat-abi=hard

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

# NEON is not enabled in vectors.s
COPS7 = -mfpu=vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

# NEON is not enabled in vectors.s
COPS8 = -mfpu=vfpv4 -march=armv8-a -mtune=cortex-a53
COPS8 += -DRPI3
COPS8 += $(COPS_COMMON)

LIB6 += -L/usr/lib/gcc/arm-none-eabi/4.9.3/fpu
LIB6 += -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv6zk/arm1176jzf-s/hardfp/vfp  

LIB7 += -L/usr/lib/gcc/arm-none-eabi/4.9.3/fpu
LIB7 += -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv7-a/cortex-a7/hardfp/vfpv4 

LIB8 += -L/usr/lib/gcc/arm-none-eabi/4.9.3/fpu
LIB8 += -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv8-a/cortex-a53/hardfp/fp-armv8/

SOURCE = ./

BUILD = build/
BUILD7 = build7/
BUILD8 = build8/

C_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
C_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS7 += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
C_OBJECTS8 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD8)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS8 += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD8)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

BUILD_DIRS := $(addprefix build/,$(SRCDIR))
BUILD7_DIRS := $(addprefix build7/,$(SRCDIR))
BUILD8_DIRS := $(addprefix build8/,$(SRCDIR))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)
OBJECTS7 := $(ASM_OBJECTS7) $(C_OBJECTS7)
OBJECTS8 := $(ASM_OBJECTS8) $(C_OBJECTS8)

TARGET = kernel.img
TARGET7 = kernel7.img
TARGET8 = kernel8.img

LIST = kernel.list
LIST7 = kernel7.list
LIST8 = kernel8.list

MAP = kernel.map
MAP7 = kernel7.map
MAP8 = kernel8.map

LINKER = firmware/memmap

define compile-objects6
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(ARMGNU)-gcc $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(ARMGNU)-g++ -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti $(COPS) -c $$< -o $$@	
endef

define compile-objects7
$(BUILD7)$1/%.o: $(SOURCE)$1/%.c
	$(ARMGNU)-gcc $(COPS7) -c $$< -o $$@
	
$(BUILD7)$1/%.o: $(SOURCE)$1/%.cpp
	$(ARMGNU)-g++ -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti $(COPS7) -c $$< -o $$@		
endef

define compile-objects8
$(BUILD8)$1/%.o: $(SOURCE)$1/%.c
	$(ARMGNU)-gcc $(COPS8) -c $$< -o $$@
	
$(BUILD8)$1/%.o: $(SOURCE)$1/%.cpp
	$(ARMGNU)-g++ -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti $(COPS8) -c $$< -o $$@		
endef

THISDIR = $(CURDIR)

all : builddirs prerequisites $(TARGET) $(TARGET7) # $(TARGET8)
	
.PHONY: clean builddirs

buildlibs:
	cd .. && ./makeall-lib.sh && cd $(THISDIR)

builddirs:
	@mkdir -p $(BUILD_DIRS) $(BUILD7_DIRS) # $(BUILD8_DIRS)

clean:
	rm -rf $(BUILD_DIRS) $(BUILD7_DIRS) $(BUILD8_DIRS)
	rm -f $(TARGET) $(TARGET7) $(TARGET8)
	rm -f $(BUILD)*.elf $(BUILD7)*.elf $(BUILD8)*.elf
	rm -f $(MAP) $(MAP7) $(MAP8)
	rm -f $(LIST) $(LIST7) $(LIST8)

# Build kernel.img

$(BUILD)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD)vectors.o
	
$(BUILD)main.elf : Makefile $(LINKER) $(BUILD)vectors.o $(OBJECTS) $(LIB6DEP)
	$(ARMGNU)-ld $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIB6) $(LDLIBS) -lgcc
	$(ARMGNU)-objdump -D $(BUILD)main.elf | $(ARMGNU)-c++filt > $(LIST)

$(TARGET) : $(BUILD)main.elf
	$(ARMGNU)-objcopy $(BUILD)main.elf -O binary $(TARGET)

# Build kernel7.img

$(BUILD7)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS7) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD7)vectors.o
	
$(BUILD7)main.elf : Makefile $(LINKER) $(BUILD7)vectors.o $(OBJECTS7) $(LIB7DEP)
	$(ARMGNU)-ld $(BUILD7)vectors.o $(OBJECTS7) -Map $(MAP7) -T $(LINKER) -o $(BUILD7)main.elf $(LIB7) $(LDLIBS) -lgcc
	$(ARMGNU)-objdump -D $(BUILD7)main.elf | $(ARMGNU)-c++filt > $(LIST7)

$(TARGET7) : $(BUILD7)main.elf
	$(ARMGNU)-objcopy $(BUILD7)main.elf -O binary $(TARGET7)
	
# Build kernel8.img

$(BUILD8)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS8) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD8)vectors.o
	
$(BUILD8)main.elf : Makefile $(LINKER) $(BUILD8)vectors.o $(OBJECTS8) $(LIB7DEP)
	$(ARMGNU)-ld $(BUILD8)vectors.o $(OBJECTS8) -Map $(MAP8) -T $(LINKER) -o $(BUILD8)main.elf $(LIB8) $(LDLIBS) -lgcc
	$(ARMGNU)-objdump -D $(BUILD8)main.elf | $(ARMGNU)-c++filt > $(LIST8)

$(TARGET8) : $(BUILD8)main.elf
	$(ARMGNU)-objcopy $(BUILD8)main.elf -O binary $(TARGET8)	

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects6,$(bdir))))
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects7,$(bdir))))

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects8,$(bdir))))