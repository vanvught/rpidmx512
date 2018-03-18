PREFIX ?=
DEF ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

INCLUDES := -I./include
INCLUDES += $(addprefix -I,$(EXTRA_INCLUDES))

DEFINES := $(addprefix -D,$(DEFINES))

COPS = $(DEF) $(DEFINES)
COPS += $(INCLUDES)
COPS += -Wall -Werror -O3

SOURCE = ./src

CURR_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME := $(patsubst lib-%,%,$(CURR_DIR))

BUILD = build_linux/

OBJECTS := $(patsubst $(SOURCE)/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/*.c))
OBJECTS += $(patsubst $(SOURCE)/%.cpp,$(BUILD)%.o,$(wildcard $(SOURCE)/*.cpp))
OBJECTS += $(patsubst $(SOURCE)/linux/%.c,$(BUILD)linux/%.o,$(wildcard $(SOURCE)/linux/*.c))
OBJECTS += $(patsubst $(SOURCE)/linux/%.cpp,$(BUILD)linux/%.o,$(wildcard $(SOURCE)/linux/*.cpp))

TARGET = lib_linux/lib$(LIB_NAME).a 

LIST = lib_linux.list

all : builddirs $(TARGET)

.PHONY: clean builddirs

builddirs:
	@mkdir -p $(BUILD)linux lib_linux

clean :
	rm -rf $(BUILD)
	rm -f $(TARGET)	
	rm -f $(LIST)	

$(BUILD)%.o: $(SOURCE)/%.c
	$(CC) $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)/%.cpp
	$(CPP) $(COPS) -fno-rtti -std=c++11 $< -c -o $@
	
$(BUILD)linux/%.o: $(SOURCE)/linux/%.c
	$(CC) $(COPS) $< -c -o $@
	
$(BUILD)linux/%.o: $(SOURCE)/linux/%.cpp
	$(CPP) $(COPS) -fno-rtti -std=c++11 $< -c -o $@	
	
$(TARGET): Makefile.Linux $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -D $(TARGET) | $(PREFIX)c++filt > $(LIST)
