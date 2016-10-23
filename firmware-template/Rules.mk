ARMGNU ?= arm-none-eabi

LIBS += hal uuid ff11 emmc fb bob bcm2835

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
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -ffreestanding -mhard-float -mfloat-abi=hard

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

# NEON is not enabled in vectors.s
COPS7 = -mfpu=vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

LIB6 += -L/usr/lib/gcc/arm-none-eabi/4.9.3/fpu
LIB6 += -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv6zk/arm1176jzf-s/hardfp/vfp  

LIB7 += -L/usr/lib/gcc/arm-none-eabi/4.9.3/fpu
LIB7 += -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv7-a/cortex-a7/hardfp/vfpv4 

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

THISDIR = $(CURDIR)

all : builddirs $(TARGET) $(TARGET7)
	
.PHONY: clean builddirs

buildlibs:
	cd .. && ./makeall-lib.sh && cd $(THISDIR)

builddirs:
	@mkdir -p $(BUILD_DIRS) $(BUILD7_DIRS)

clean :
	rm -rf $(BUILD_DIRS) $(BUILD7_DIRS)
	rm -f $(TARGET) $(TARGET7)
	rm -f $(BUILD)*.elf $(BUILD7)*.elf
	rm -f $(MAP) $(MAP7)
	rm -f $(LIST) $(LIST7)
#	cd .. && ./makeall-lib.sh clean && cd $(THISDIR) 

# Build kernel.img

$(BUILD)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD)vectors.o
	
$(BUILD)main.elf : Makefile $(LINKER) $(BUILD)vectors.o $(OBJECTS) $(LIB6DEP)
	$(ARMGNU)-ld $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIB6) $(LDLIBS) -lgcc #-lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD)main.elf > $(LIST)

$(TARGET) : $(BUILD)main.elf
	$(ARMGNU)-objcopy $(BUILD)main.elf -O binary $(TARGET)

# Build kernel7.img

$(BUILD7)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS7) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD7)vectors.o
	
$(BUILD7)%.o: $(SOURCE)firmware/%.cpp
	$(ARMGNU)-g++ -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti $(COPS7) $< -c -o $@

$(BUILD7)main.elf : Makefile $(LINKER) $(BUILD7)vectors.o $(OBJECTS7) $(LIB7DEP)
	$(ARMGNU)-ld $(BUILD7)vectors.o $(OBJECTS7) -Map $(MAP7) -T $(LINKER) -o $(BUILD7)main.elf $(LIB7) $(LDLIBS) -lgcc  #-lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD7)main.elf > $(LIST7)

$(TARGET7) : $(BUILD7)main.elf
	$(ARMGNU)-objcopy $(BUILD7)main.elf -O binary $(TARGET7)

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects6,$(bdir))))
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects7,$(bdir))))