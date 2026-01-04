$(info "lib/Rules.mk")
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

SRCDIR=src src/arm src/arm/h3 src/h3 src/debug $(EXTRA_SRCDIR)

include ../firmware-template-h3/Soc.mk
include ../firmware-template-h3/Phy.mk
include ../firmware-template-h3/Board.mk
include ../common/make/DmxNodeNodeType.mk
include ../common/make/DmxNodeOutputType.mk
include ../firmware-template-h3/Includes.mk

INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))

DEFINES:=$(addprefix -D,$(DEFINES)) $(BOARD_DEFS)
ifeq ($(NO_EXT_LED),1)
	DEFINES+=-DDO_NOT_USE_EXTERNAL_LED
endif

COPS =$(strip $(MAKE_FLAGS) $(DEFINES) $(VALIDATE_FLAGS) $(INCLUDES))
COPS+=$(strip $(ARMOPS) $(CMSISOPS))
COPS+=-Os
COPS+=-nostartfiles -ffreestanding -nostdlib
COPS+=-fstack-usage
COPS+=-ffunction-sections -fdata-sections
COPS+=-Wall -Werror -Wpedantic -Wextra -Wunused -Wsign-conversion -Wduplicated-cond -Wlogical-op
ifndef FREE_RTOS_PORTABLE
COPS+=-Wconversion
endif

include ../common/make/CppOps.mk

BUILD=build_h3/
BUILD_DIRS:=$(addprefix build_h3/,$(SRCDIR))
$(info $$BUILD_DIRS [${BUILD_DIRS}])

include ../common/make/lib/Objects.mk

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME:=$(patsubst lib-%,%,$(CURR_DIR))
TARGET =lib_h3/lib$(LIB_NAME).a 
LIST   = lib.list

$(info $$SRCDIR [${SRCDIR}])
$(info $$MAKECMDGOALS [${MAKECMDGOALS}])
$(info $$MAKE_FLAGS [${MAKE_FLAGS}])
$(info $$DEFINES [${DEFINES}])
$(info $$OBJECTS [${OBJECTS}])
$(info $$TARGET [${TARGET}])

all : builddirs $(TARGET)

.PHONY: clean builddirs
	
clean:
	rm -rf build_h3
	rm -rf lib_h3

builddirs:
	mkdir -p $(BUILD_DIRS) lib_h3 \
		$(if $(EXTRA_C_BUILD_DIRS),$(EXTRA_C_BUILD_DIRS)) \
		$(if $(EXTRA_CPP_BUILD_DIRS),$(EXTRA_CPP_BUILD_DIRS))
	
$(BUILD)%.o: %.S
	$(CC) $(COPS) -D__ASSEMBLY__ -c $< -o $@

$(BUILD)%.o: %.c
	$(CC) $(COPS) -c $< -o $@

$(BUILD)%.o: %.cpp
	$(CC) $(COPS) $(CPPOPS) -c $< -o $@
	
$(TARGET): Makefile.H3 $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -d $(TARGET) | $(PREFIX)c++filt > lib_h3/$(LIST)
