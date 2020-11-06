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

ifeq ($(findstring ENABLE_SPIFLASH,$(DEFINES)),ENABLE_SPIFLASH)
	COND=1
endif

ifeq ($(findstring NO_EMAC,$(DEFINES)),NO_EMAC)
else
	ifdef COND
		LIBS:=remoteconfig $(LIBS)
	endif
endif

ifeq ($(findstring DISPLAY_UDF,$(DEFINES)),DISPLAY_UDF)
	ifdef COND
		LIBS+=displayudf
	endif
endif

ifeq ($(findstring ARTNET_NODE,$(DEFINES)),ARTNET_NODE)
	ifdef COND
		LIBS+=artnet4 artnet artnethandlers e131 uuid
	endif
endif

ifeq ($(findstring E131_BRIDGE,$(DEFINES)),E131_BRIDGE)
	ifdef COND
		LIBS+=artnet e131 uuid
	endif
endif

RDM=
ifeq ($(findstring RDMNET_LLRP_ONLY,$(DEFINES)),RDMNET_LLRP_ONLY)
	LIBS+=artnet e131
	RDM=1
	ifneq ($(findstring uuid,$(LIBS)),uuid)
		LIBS+=uuid
	endif
endif

ifeq ($(findstring DMXSEND,$(DEFINES)),DMXSEND)
	LIBS+=dmxsend dmx
endif

ifeq ($(findstring PIXEL,$(DEFINES)),PIXEL)
	LIBS+=ws28xxdmx ws28xx tlc59711dmx tlc59711
endif

ifdef COND
	LIBS+=spiflashinstall spiflashstore spiflash
endif

ifeq ($(findstring ESP8266,$(DEFINES)),ESP8266)
	LIBS+=esp8266
	INCDIRS=../lib-network/include
else
	LIBS+=network
endif

ifeq ($(findstring LTC_READER,$(DEFINES)),LTC_READER)
	DEFINES+=ENABLE_SSD1311 ENABLE_TC1602 ENABLE_CURSOR_MODE
endif

ifeq ($(findstring rdmresponder,$(LIBS)),rdmresponder)
	RDM=1
endif

# Output 
TARGET = $(SUFFIX).img
LIST = $(SUFFIX).list
MAP = $(SUFFIX).map
BUILD=build_h3/

# Input
SOURCE = ./
FIRMWARE_DIR = ./../h3-firmware-template/
LINKER = $(FIRMWARE_DIR)memmap

LIBS+=lightset properties display device hal c++ c debug h3 arm

DEFINES:=$(addprefix -D,$(DEFINES))

ifneq ($(CONSOLE),)
	DEFINES+=-D$(CONSOLE)
endif

DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")

# The variable for the firmware include directories
INCDIRS+=../include $(wildcard ./include) $(wildcard ./*/include)
INCDIRS:=$(addprefix -I,$(INCDIRS))

$(info $$INCDIRS [${INCDIRS}])

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
LIBSDEP=$(addsuffix /lib_h3/lib, $(LIBDEP))
LIBSDEP:=$(join $(LIBSDEP), $(LIBS))
LIBSDEP:=$(addsuffix .a, $(LIBSDEP))

COPS=-DBARE_METAL -DH3 -D$(PLATFORM) $(DEFINES)
COPS+=$(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS+=-mfpu=neon-vfpv4 -mcpu=cortex-a7 -mfloat-abi=hard -mhard-float
COPS+=-nostartfiles -ffreestanding -nostdinc -nostdlib -fprefetch-loop-arrays
#COPS+=-fstack-usage
COPS+=-O2 -Wall -Werror -Wpedantic -Wextra -Wunused -Wsign-conversion  #-Wconversion

CPPOPS=-std=c++11 -Wuseless-cast -Wold-style-cast -Wnon-virtual-dtor -Wnull-dereference -fno-rtti -fno-exceptions -fno-unwind-tables

# Why does gcc not automatically select the correct path based on -m options?
PLATFORM_LIBGCC:= -L $(shell dirname `$(CC) $(COPS) -print-libgcc-file-name`)/armv7-a/cortex-a7/hardfp/vfpv4
PLATFORM_LIBGCC+= -L $(shell dirname `$(CC) $(COPS) -print-libgcc-file-name`)

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

all : clearlibs builddirs prerequisites $(TARGET)
	
.PHONY: clean builddirs

clearlibs:
	$(MAKE) -f Makefile.H3 clean --directory=../lib-display
	$(MAKE) -f Makefile.H3 clean --directory=../lib-h3
	$(MAKE) -f Makefile.H3 clean --directory=../lib-hal
	$(MAKE) -f Makefile.H3 clean --directory=../lib-remoteconfig
	$(MAKE) -f Makefile.H3 clean --directory=../lib-spiflashstore
ifdef RDM
	$(MAKE) -f Makefile.H3 clean --directory=../lib-rdm
	$(MAKE) -f Makefile.H3 clean --directory=../lib-rdmsensor
	$(MAKE) -f Makefile.H3 clean --directory=../lib-rdmsubdevice
endif	

builddirs:
	mkdir -p $(BUILD_DIRS)
	[ -f generate_sofware_version_id.sh ] && chmod u+x generate_sofware_version_id.sh || true

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
	rm -f $(TARGET).gz
	rm -f $(MAP)
	rm -f $(LIST)
	rm -f $(SUFFIX).uImage
	rm -f $(SUFFIX).uImage.gz
	rm -f build$(BUILD_TXT).txt
	for d in $(LIBDEP); \
		do                               \
			$(MAKE) -f Makefile.H3 clean --directory=$$d;       \
		done

#
# Build libraries
#
$(LIBSDEP):
	for d in $(LIBDEP); \
		do                               \
			$(MAKE) -f Makefile.H3 'PLATFORM=$(PLATFORM)' 'MAKE_FLAGS=$(DEFINES)' --directory=$$d;       \
		done

# Build uImage

$(BUILD_DIRS) :
	mkdir -p $(BUILD_DIRS)

$(BUILD)vectors.o : $(FIRMWARE_DIR)/vectors.S
	$(AS) $(COPS) -D__ASSEMBLY__ -c $(FIRMWARE_DIR)/vectors.S -o $(BUILD)vectors.o
	
$(BUILD)main.elf: Makefile.H3 $(LINKER) $(BUILD)vectors.o $(OBJECTS) $(LIBSDEP)
	$(LD) $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIBH3) $(LDLIBS) $(PLATFORM_LIBGCC) -lgcc
	$(PREFIX)objdump -D $(BUILD)main.elf | $(PREFIX)c++filt > $(LIST)
	$(PREFIX)size -A $(BUILD)main.elf

$(TARGET) : $(BUILD)main.elf 
	$(PREFIX)objcopy $(BUILD)main.elf -O binary $(TARGET)	
	$(GZIP) -n -c $(TARGET) > $(TARGET).gz 
	mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C none -a 0x40000000 -d $(TARGET) $(SUFFIX).uImage
	mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C gzip -a 0x40000000 -d $(TARGET).gz $(SUFFIX).uImage.gz

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
