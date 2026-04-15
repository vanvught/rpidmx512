$(info "Rules.mk")
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
NO_EXT_LED?=

SUFFIX=orangepi_zero
BUILD_TXT=0

ifeq ($(findstring ORANGE_PI_ONE,$(PLATFORM)),ORANGE_PI_ONE)
	SUFFIX=orangepi_one
	BUILD_TXT=1
endif

TARGET=$(SUFFIX).img
LIST=$(SUFFIX).list
MAP=$(SUFFIX).map
BUILD=build_h3/

SOURCE=./
FIRMWARE_DIR=./../firmware-template-h3/
LINKER=$(FIRMWARE_DIR)memmap

PROJECT=$(notdir $(patsubst %/,%,$(CURDIR)))
$(info $$PROJECT [${PROJECT}])
	
DEFINES:=$(addprefix -D,$(DEFINES))

$(info $$DEFINES [${DEFINES}])

ifneq ($(CONSOLE),)
	DEFINES+=-D$(CONSOLE)
endif

ifneq ($(findstring CONFIG_STORE_USE_SPI,$(DEFINES)), CONFIG_STORE_USE_SPI)
	DEFINES+=-DCONFIG_STORE_USE_SPI
endif

ifeq ($(findstring ARTNET_VERSION=4,$(DEFINES)),ARTNET_VERSION=4)
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(DEFINES)),ARTNET_HAVE_DMXIN)
		DEFINES+=-DE131_HAVE_DMXIN
	endif
endif

DEFINES+=-DCONFIG_NETWORK_MEMORY_BLOCKS=32

include ../firmware-template-h3/Soc.mk
include ../firmware-template-h3/Phy.mk
include ../firmware-template-h3/Board.mk
include ../firmware-template/libs.mk
include ../common/make/DmxNodeNodeType.mk
include ../common/make/DmxNodeOutputType.mk
include ../firmware-template-h3/Includes.mk
include ../common/make/Timestamp.mk

LIBS+=h3 clib arm

LIBH3=$(addprefix -L../lib-,$(LIBS))
LIBH3:=$(addsuffix /lib_h3, $(LIBH3))

LDLIBS:=$(addprefix -l,$(LIBS))

LIBDEP=$(addprefix ../lib-,$(LIBS))

$(info $$LIBDEP [${LIBDEP}])

DEFINES:=$(BOARD_DEFS) $(DEFINES)

COPS =$(strip $(DEFINES) $(MAKE_FLAGS) $(INCLUDES) $(LIBINCDIRS))
COPS+=$(strip $(ARMOPS) $(CMSISOPS))
COPS+=-O2 -fprefetch-loop-arrays
COPS+=-nostartfiles -ffreestanding -nostdlib
COPS+=-fstack-usage
COPS+=-ffunction-sections -fdata-sections
COPS+=-Wall -Werror -Wpedantic -Wextra -Wunused -Wsign-conversion -Wconversion -Wduplicated-cond -Wlogical-op
COPS+=--specs=nosys.specs

include ../common/make/CppOps.mk

LDOPS=--gc-sections --print-gc-sections --print-memory-usage

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

.PHONY: libdep $(LIBDEP)

libdep: $(LIBDEP)

$(LIBDEP):
	$(MAKE) -f Makefile.H3 $(MAKECMDGOALS) 'PROJECT=${PROJECT}' 'PLATFORM=$(PLATFORM)' 'CONSOLE=$(CONSOLE)' 'MAKE_FLAGS=$(DEFINES)' -C $@ 

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
