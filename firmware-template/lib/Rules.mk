PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

INCLUDES := -I../include -I./include
INCLUDES += $(addprefix -I,$(EXTRA_INCLUDES))

DEFINES := $(addprefix -D,$(DEFINES))

COPS_COMMON = -DBARE_METAL $(DEFINES)
COPS_COMMON += $(INCLUDES)
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -ffreestanding -nostdinc -nostdlib -mhard-float -mfloat-abi=hard -fno-exceptions -fno-unwind-tables #-fstack-usage

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

SOURCE = ./src

CURR_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME := $(patsubst lib-%,%,$(CURR_DIR))

BUILD = build/
BUILD7 = build7/

OBJECTS := $(patsubst $(SOURCE)/%.S,$(BUILD)%.o,$(wildcard $(SOURCE)/*.S))
OBJECTS += $(patsubst $(SOURCE)/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/*.c)) 
OBJECTS += $(patsubst $(SOURCE)/%.cpp,$(BUILD)%.o,$(wildcard $(SOURCE)/*.cpp)) 
OBJECTS += $(patsubst $(SOURCE)/rpi/%.S,$(BUILD)rpi%.o,$(wildcard $(SOURCE)/rpi/*.S))
OBJECTS += $(patsubst $(SOURCE)/rpi/%.c,$(BUILD)rpi/%.o,$(wildcard $(SOURCE)/rpi/*.c))
OBJECTS += $(patsubst $(SOURCE)/rpi/%.cpp,$(BUILD)rpi/%.o,$(wildcard $(SOURCE)/rpi/*.cpp))

OBJECTS7 := $(patsubst $(SOURCE)/%.S,$(BUILD7)%.o,$(wildcard $(SOURCE)/*.S))
OBJECTS7 += $(patsubst $(SOURCE)/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)/*.c)) 
OBJECTS7 += $(patsubst $(SOURCE)/%.cpp,$(BUILD7)%.o,$(wildcard $(SOURCE)/*.cpp)) 
OBJECTS7 += $(patsubst $(SOURCE)/rpi/%.S,$(BUILD7)rpi/%.o,$(wildcard $(SOURCE)/rpi/*.S))
OBJECTS7 += $(patsubst $(SOURCE)/rpi/%.c,$(BUILD7)rpi/%.o,$(wildcard $(SOURCE)/rpi/*.c))
OBJECTS7 += $(patsubst $(SOURCE)/rpi/%.cpp,$(BUILD7)rpi/%.o,$(wildcard $(SOURCE)/rpi/*.cpp))

TARGET = lib/lib$(LIB_NAME).a 
TARGET7 = lib7/lib$(LIB_NAME).a

LIST = lib.list
LIST7 = lib7.list

all : builddirs $(TARGET) $(TARGET7)

.PHONY: clean builddirs

builddirs:
	@mkdir -p $(BUILD)rpi lib $(BUILD7)rpi lib7

clean :
	rm -rf $(BUILD) $(BUILD7)
	rm -f $(TARGET) $(TARGET7)	
	rm -f $(LIST) $(LIST7)
	rm -rf lib lib7

# ARM v6
$(BUILD)%.o: $(SOURCE)/%.c
	$(CC) $(COPS) $< -c -o $@
	
$(BUILD)rpi/%.o: $(SOURCE)/rpi/%.c
	$(CC) $(COPS) $< -c -o $@	

$(BUILD)%.o: $(SOURCE)/%.cpp
	$(CPP) $(COPS) -fno-rtti -std=c++11 $< -c -o $@

$(BUILD)rpi/%.o: $(SOURCE)/rpi/%.cpp
	$(CPP) $(COPS) -fno-rtti -std=c++11 $< -c -o $@

$(BUILD)%.o: $(SOURCE)/%.S
	$(AS) $(COPS) -D__ASSEMBLY__ $< -c -o $@	
	
$(BUILD)rpi/%.o: $(SOURCE)/rpi/%.S
	$(AS) $(COPS) -D__ASSEMBLY__ $< -c -o $@		

$(TARGET): Makefile $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -D $(TARGET) | $(PREFIX)c++filt > $(LIST)
	
# ARM v7	
$(BUILD7)%.o: $(SOURCE)/%.c
	$(CC) $(COPS7) $< -c -o $@

$(BUILD7)rpi/%.o: $(SOURCE)/rpi/%.c
	$(CC) $(COPS7) $< -c -o $@
	
$(BUILD7)%.o: $(SOURCE)/%.cpp
	$(CPP) $(COPS7) -fno-rtti -std=c++11 $< -c -o $@		

$(BUILD7)rpi/%.o: $(SOURCE)/rpi/%.cpp
	$(CPP) $(COPS7) -fno-rtti -std=c++11 $< -c -o $@	

$(BUILD7)%.o: $(SOURCE)/%.S
	$(AS) $(COPS7) -D__ASSEMBLY__ $< -c -o $@	
	
$(BUILD7)rpi/%.o: $(SOURCE)/rpi/%.S
	$(AS) $(COPS7) -D__ASSEMBLY__ $< -c -o $@		

$(TARGET7): Makefile $(OBJECTS7)
	$(AR) -r $(TARGET7) $(OBJECTS7)
	$(PREFIX)objdump -D $(TARGET7) | $(PREFIX)c++filt > $(LIST7)
