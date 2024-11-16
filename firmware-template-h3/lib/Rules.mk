PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

$(info [${CURDIR}])

PLATFORM?=ORANGE_PI
CONSOLE?=
NO_EXT_LED?=

ifeq ($(findstring ORANGE_PI_ONE,$(PLATFORM)),ORANGE_PI_ONE)
else
endif

SRCDIR=src src/arm src/arm/h3 src/h3 $(EXTRA_SRCDIR)

#ifeq ($(findstring NDEBUG,$(DEFINES)),NDEBUG)
#else
SRCDIR+=src/debug
#endif

$(info [${SRCDIR}])

INCLUDES:=-I./include -I../include -I../lib-device/include -I../lib-configstore/include -I../lib-hal/include -I../lib-debug/include -I../lib-h3/include -I../lib-arm/include 
INCLUDES+=-I../lib-flash/include -I../lib-flashcodeinstall/include
INCLUDES+=-I../lib-h3/CMSIS/Core_A/Include
INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))

DEFINES:=-D$(PLATFORM) $(addprefix -D,$(DISPLAYS)) $(addprefix -D,$(DEFINES))

ifneq ($(findstring _TIME_STAMP_YEAR_,$(DEFINES)), _TIME_STAMP_YEAR_)
	DEFINES+=-D_TIME_STAMP_YEAR_=$(shell date  +"%Y") -D_TIME_STAMP_MONTH_=$(shell date  +"%-m") -D_TIME_STAMP_DAY_=$(shell date  +"%-d")
endif

include ../firmware-template-h3/Common.mk

ifneq ($(findstring CONFIG_STORE_USE_SPI,$(DEFINES)), CONFIG_STORE_USE_SPI)
	DEFINES+=-DCONFIG_STORE_USE_SPI
endif

ifneq ($(CONSOLE),)
	ifeq ($(findstring $(CONSOLE),$(MAKE_FLAGS)), $(CONSOLE))
	else
		DEFINES+=-D$(CONSOLE)
	endif
endif

ifeq ($(NO_EXT_LED),1)
	DEFINES+=-DDO_NOT_USE_EXTERNAL_LED
endif

ifeq ($(findstring ARTNET_VERSION=4,$(DEFINES)),ARTNET_VERSION=4)
	ifeq ($(findstring ARTNET_HAVE_DMXIN,$(DEFINES)),ARTNET_HAVE_DMXIN)
		DEFINES+=-DE131_HAVE_DMXIN
	endif
endif

$(info $$DEFINES [${DEFINES}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

COPS=-DBARE_METAL -DH3 $(DEFINES) $(MAKE_FLAGS) $(INCLUDES)
COPS+=-mfpu=neon-vfpv4 -mcpu=cortex-a7 -mfloat-abi=hard -mhard-float
COPS+=-nostartfiles -ffreestanding -nostdlib 
COPS+=-Os -Wall -Werror -Wextra -Wpedantic -Wunused -Wsign-conversion -Wconversion 
COPS+=-Wduplicated-cond -Wlogical-op -Wduplicated-branches
COPS+=-ffunction-sections -fdata-sections

CPPOPS=-std=c++20 -Wuseless-cast -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual -Wnull-dereference -fno-rtti -fno-exceptions -fno-unwind-tables
CPPOPS+=-fno-threadsafe-statics

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME:=$(patsubst lib-%,%,$(CURR_DIR))

BUILD=build_h3/
BUILD_DIRS:=$(addprefix build_h3/,$(SRCDIR))
$(info $$BUILD_DIRS [${BUILD_DIRS}])

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS)

TARGET=lib_h3/lib$(LIB_NAME).a 
$(info $$TARGET [${TARGET}])

LIST = lib.list

define compile-objects
$(BUILD)$1/%.o: $1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $1/%.cpp
	$(CPP) $(COPS) $(CPPOPS)  -c $$< -o $$@
	
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
	
