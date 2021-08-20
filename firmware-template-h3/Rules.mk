PREFIX ?= arm-none-eabi-

CC	 = $(PREFIX)gcc
CPP	 = $(PREFIX)g++
AS	 = $(CC)
LD	 = $(PREFIX)ld
AR	 = $(PREFIX)ar
GZIP = gzip

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

ifeq ($(findstring NO_EMAC,$(DEFINES)),NO_EMAC)
else
	ifdef COND
		LIBS:=remoteconfig $(LIBS)
	endif
endif

ifeq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
	ifdef COND
		LIBS+=artnet4 artnet e131 uuid
	endif
endif

ifeq ($(findstring NODE_E131,$(DEFINES)),NODE_E131)
	ifdef COND
		ifneq ($(findstring e131,$(LIBS)),e131)
			LIBS+=e131
		endif
		ifneq ($(findstring artnet,$(LIBS)),artnet)
			LIBS+=artnet
		endif
		LIBS+=uuid
	endif
endif

ifeq ($(findstring NODE_RDMNET_LLRP_ONLY,$(DEFINES)),NODE_RDMNET_LLRP_ONLY)
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131
	endif
	ifneq ($(findstring uuid,$(LIBS)),uuid)
		LIBS+=uuid
	endif
	ifneq ($(findstring artnet,$(LIBS)),artnet)
		LIBS+=artnet
	endif
endif

ifeq ($(findstring OUTPUT_DMX_SEND,$(DEFINES)),OUTPUT_DMX_SEND)
	LIBS+=dmxsend dmx
endif

ifeq ($(findstring OUTPUT_DDP_PIXEL_MULTI,$(DEFINES)),OUTPUT_DDP_PIXEL_MULTI)
	LIBS+=ws28xxdmx ws28xx jamstapl
else
	ifeq ($(findstring OUTPUT_DMX_PIXEL_MULTI,$(DEFINES)),OUTPUT_DMX_PIXEL_MULTI)
		LIBS+=ws28xxdmx ws28xx jamstapl
	else
		ifeq ($(findstring OUTPUT_DMX_PIXEL,$(DEFINES)),OUTPUT_DMX_PIXEL)
			LIBS+=ws28xxdmx ws28xx tlc59711dmx tlc59711
		endif
	endif
endif

ifdef COND
	LIBS+=spiflashinstall spiflashstore spiflash
endif

ifeq ($(findstring NODE_LTC_SMPTE,$(DEFINES)),NODE_LTC_SMPTE)
	DEFINES+=ENABLE_SSD1311 ENABLE_TC1602 ENABLE_CURSOR_MODE
endif

ifeq ($(findstring rdmresponder,$(LIBS)),rdmresponder)
	LIBS+=rdm rdmsensor rdmsubdevice
endif

LIBS+=network properties

ifeq ($(findstring DISPLAY_UDF,$(DEFINES)),DISPLAY_UDF)
	ifdef COND
		LIBS+=displayudf
	endif
endif

LIBS+=lightset display device hal c++ h3 debug c arm

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

DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")

# The variable for the firmware include directories
INCDIRS+=../include $(wildcard ./include) $(wildcard ./*/include)
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
COPS+=-nostartfiles -ffreestanding -nostdinc -nostdlib -fprefetch-loop-arrays
COPS+=-O2 -Wall -Werror -Wpedantic -Wextra -Wunused -Wsign-conversion  -Wconversion
COPS+=-Wduplicated-cond -Wlogical-op #-Wduplicated-branches
#COPS+=-fstack-usage

CPPOPS=-std=c++11 -Wuseless-cast -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual -Wnull-dereference -fno-rtti -fno-exceptions -fno-unwind-tables

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
	[ -f generate_sofware_version_id.sh ] && chmod u+x generate_sofware_version_id.sh || true

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
	$(LD) $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIBH3) $(LDLIBS) $(PLATFORM_LIBGCC) -lgcc 
	$(PREFIX)objdump -D $(BUILD)main.elf | $(PREFIX)c++filt > $(LIST)
	$(PREFIX)size -A -x $(BUILD)main.elf

$(TARGET) : $(BUILD)main.elf 
	$(PREFIX)objcopy $(BUILD)main.elf -O binary $(TARGET)	
	$(GZIP) -n -c $(TARGET) > $(TARGET).gz 
	mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C none -a 0x40000000 -d $(TARGET) $(SUFFIX).uImage
	mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C gzip -a 0x40000000 -d $(TARGET).gz $(SUFFIX).uImage.gz

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
