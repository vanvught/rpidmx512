PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

PLATFORM?=ORANGE_PI
CONSOLE?=
NO_EXT_LED?=

$(info [${CURDIR}])

ifeq ($(findstring ORANGE_PI_ONE,$(PLATFORM)),ORANGE_PI_ONE)
else
endif

SRCDIR = src src/h3 $(EXTRA_SRCDIR)

INCLUDES:= -I./include -I../include -I../lib-ledblink/include -I../lib-hal/include -I../lib-debug/include -I../lib-h3/include -I../lib-arm/include 
INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))

DEFINES:=-D$(PLATFORM) $(addprefix -D,$(DISPLAYS)) $(addprefix -D,$(DEFINES))
DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")

ifneq ($(CONSOLE),)
	ifeq ($(findstring $(CONSOLE),$(MAKE_FLAGS)), $(CONSOLE))
	else
		DEFINES+=-D$(CONSOLE)
	endif
endif

ifeq ($(NO_EXT_LED),1)
	DEFINES+=-DDO_NOT_USE_EXTERNAL_LED
endif

ifeq ($(findstring ENABLE_SPIFLASH,$(MAKE_FLAGS)), ENABLE_SPIFLASH)
	DEFINES+=-DREMOTE_CONFIG
else
	ifeq ($(findstring ORANGE_PI_ONE,$(PLATFORM)),ORANGE_PI_ONE)
	else
		DEFINES+=-DREMOTE_CONFIG
	endif
endif	

$(info $$DEFINES [${DEFINES}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

COPS=-DBARE_METAL -DH3 $(DEFINES) $(MAKE_FLAGS) $(INCLUDES)
COPS+=-mfpu=neon-vfpv4 -mcpu=cortex-a7 -mhard-float -mfloat-abi=hard
COPS+=-O2 -Wall -Werror -Wunused #-Wpedantic #-Wextra  #-Wconversion #-Wcast-align
COPS+= -nostartfiles -ffreestanding -nostdinc -nostdlib -fno-exceptions -fno-unwind-tables -fprefetch-loop-arrays

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME:=$(patsubst lib-%,%,$(CURR_DIR))

BUILD = build_h3/
BUILD_DIRS:=$(addprefix build_h3/,$(SRCDIR))
$(info $$BUILD_DIRS [${BUILD_DIRS}])

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS)

TARGET=lib_h3/lib$(LIB_NAME).a 
$(info $$TARGET [${TARGET}])

LIST = lib.list

#-Wuseless-cast

define compile-objects
$(BUILD)$1/%.o: $1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.cpp
	$(CPP) $(COPS) -std=c++11 -fno-rtti -Wold-style-cast -Wnon-virtual-dtor -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.S
	$(CC) $(COPS) -D__ASSEMBLY__ -c $$< -o $$@	
endef

all : builddirs $(TARGET)

.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD_DIRS)
	mkdir -p lib_h3

clean:
	rm -rf build_h3
	rm -rf lib_h3
	
$(BUILD_DIRS) :	
	mkdir -p $(BUILD_DIRS)
	mkdir -p lib_h3
	
$(TARGET): Makefile.H3 $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -d $(TARGET) | $(PREFIX)c++filt > lib_h3/$(LIST)
	
$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
	
