PREFIX ?= arm-none-eabi-

CC	 = $(PREFIX)gcc
CPP	 = $(PREFIX)g++
AS	 = $(CC)
LD	 = $(PREFIX)ld
AR	 = $(PREFIX)ar
GZIP = gzip

$(info [${CURDIR}])

PLATFORM?=ORANGE_PI
CONSOLE?=

SUFFIX=orangepi_zero
BUILD_TXT=0

$(info [${CURDIR}])

ifeq ($(findstring ORANGE_PI_ONE,$(PLATFORM)),ORANGE_PI_ONE)
	SUFFIX=orangepi_one
	BUILD_TXT=1
endif

COND=
ifeq ($(findstring ORANGE_PI,$(PLATFORM)),ORANGE_PI)
	COND=1
endif

# Output 
TARGET=$(SUFFIX).img
LIST=$(SUFFIX).list
MAP=$(SUFFIX).map
BUILD=build_h3/

# Input
SOURCE=./
FIRMWARE_DIR=./../firmware-template-h3/
LINKER=$(FIRMWARE_DIR)memmap
	
DEFINES:=$(addprefix -D,$(DEFINES))

ifneq ($(CONSOLE),)
	DEFINES+=-D$(CONSOLE)
endif

ifneq ($(findstring _TIME_STAMP_YEAR_,$(DEFINES)), _TIME_STAMP_YEAR_)
	DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")
endif

include ../firmware-template-h3/Common.mk

ifneq ($(findstring CONFIG_STORE_USE_SPI,$(DEFINES)), CONFIG_STORE_USE_SPI)
	DEFINES+=-DCONFIG_STORE_USE_SPI
endif

ifeq ($(findstring ARTNET_VERSION=4,$(DEFINES)),ARTNET_VERSION=4)
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(DEFINES)),ARTNET_HAVE_DMXIN)
		DEFINES+=-DE131_HAVE_DMXIN
	endif
endif

include ../firmware-template/libs.mk

LIBS+=h3 clib arm

# The variable for the firmware include directories
INCDIRS+=../include $(wildcard ./include) $(wildcard ./*/include)  ../firmware-template-h3/include ../lib-h3/CMSIS/Core_A/Include -I../lib-flashcodeinstall/include
INCDIRS:=$(addprefix -I,$(INCDIRS))

# The variable for the libraries include directory
LIBINCDIRS:=$(addprefix -I../lib-,$(LIBS))
LIBINCDIRS+=$(addsuffix /include, $(LIBINCDIRS))

# The variables for the ld -L flag
LIBH3=$(addprefix -L../lib-,$(LIBS))
LIBH3:=$(addsuffix /lib_h3, $(LIBH3))

# The variable for the ld -l flag 
LDLIBS:=$(addprefix -l,$(LIBS))

# The variables for the dependency check 
LIBDEP=$(addprefix ../lib-,$(LIBS))

$(info [${LIBDEP}])

COPS=-DBARE_METAL -DH3 -D$(PLATFORM) $(DEFINES)
COPS+=$(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS+=-mfpu=neon-vfpv4 -mcpu=cortex-a7 -mfloat-abi=hard -mhard-float
COPS+=-nostartfiles -ffreestanding -nostdlib -fprefetch-loop-arrays
COPS+=-O2 -Wall -Werror -Wpedantic -Wextra -Wunused -Wsign-conversion  -Wconversion
COPS+=-Wduplicated-cond -Wlogical-op -Wduplicated-branches
COPS+=-ffunction-sections -fdata-sections

CPPOPS=-std=c++20 -Wuseless-cast -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual -Wnull-dereference -fno-rtti -fno-exceptions -fno-unwind-tables
CPPOPS+=-fno-threadsafe-statics

LDOPS=--gc-sections --print-gc-sections

# Why does gcc not automatically select the correct path based on -m options?
PLATFORM_LIBGCC:=-L $(shell dirname `$(CC) $(COPS) -print-libgcc-file-name`)/armv7-a/cortex-a7/hardfp/vfpv4
PLATFORM_LIBGCC+=-L $(shell dirname `$(CC) $(COPS) -print-libgcc-file-name`)

$(info $$PLATFORM_LIBGCC [${PLATFORM_LIBGCC}])

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS+=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

BUILD_DIRS:=$(addprefix $(BUILD),$(SRCDIR))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS)

define compile-objects
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) $(COPS) $(CPPOPS) -c $$< -o $$@	

$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.S
	$(CC) $(COPS) -D__ASSEMBLY__ -c $$< -o $$@
endef

all : builddirs prerequisites $(TARGET)
	
.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD_DIRS)

.PHONY:  clean

clean: $(LIBDEP)
	rm -rf $(BUILD)
	rm -f $(TARGET)
	rm -f $(TARGET).gz
	rm -f $(MAP)
	rm -f $(LIST)
	rm -f $(SUFFIX).uImage
	rm -f $(SUFFIX).uImage.gz
	rm -f build$(BUILD_TXT).txt
	rm -f include/sofware_version_id.h
	
#
# Libraries
#

.PHONY: libdep $(LIBDEP)

libdep: $(LIBDEP)

$(LIBDEP):
	$(MAKE) -f Makefile.H3 $(MAKECMDGOALS) 'PLATFORM=$(PLATFORM)' 'MAKE_FLAGS=$(DEFINES)' -C $@ 

# Build uImage

$(BUILD_DIRS) :
	mkdir -p $(BUILD_DIRS)

$(BUILD)vectors.o : $(FIRMWARE_DIR)/vectors.S
	$(AS) $(COPS) -D__ASSEMBLY__ -c $(FIRMWARE_DIR)/vectors.S -o $(BUILD)vectors.o
	
$(BUILD)main.elf: Makefile.H3 $(LINKER) $(BUILD)vectors.o $(OBJECTS) $(LIBDEP)
	$(LD) $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) $(LDOPS) -o $(BUILD)main.elf $(LIBH3) $(LDLIBS) $(PLATFORM_LIBGCC) -lgcc 
	$(PREFIX)objdump -D $(BUILD)main.elf | $(PREFIX)c++filt > $(LIST)
	$(PREFIX)size -A -x $(BUILD)main.elf

$(TARGET) : $(BUILD)main.elf 
	$(PREFIX)objcopy $(BUILD)main.elf -O binary $(TARGET)	
	$(GZIP) -n -c $(TARGET) > $(TARGET).gz 
	mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C none -a 0x40000000 -d $(TARGET) $(SUFFIX).uImage
	mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C gzip -a 0x40000000 -d $(TARGET).gz $(SUFFIX).uImage.gz

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
