PREFIX ?=

CC	=$(PREFIX)gcc
CPP	=$(PREFIX)g++
AS	=$(CC)
LD	=$(PREFIX)ld
AR	=$(PREFIX)ar

INCLUDES:=-I./include -I../lib-debug/include
INCLUDES+=$(addprefix -I,$(EXTRA_INCLUDES))

DEFINES:=$(addprefix -D,$(DEFINES))

ifeq ($(findstring lib-bob,$(INCLUDES)),lib-bob)
	ifneq ($(findstring lib-i2c,$(INCLUDES)),lib-i2c)
		INCLUDES +=-I../lib-i2c/include
	endif
endif	

ifneq (, $(shell which /opt/vc/bin/vcgencmd))
	BCM2835 = ./../lib-bcm2835_raspbian
	ifneq "$(wildcard $(BCM2835) )" ""
		INCLUDES +=-I../lib-bcm2835_raspbian/include
	endif
	DEFINES+=-DRASPPI
endif
 
COPS=$(DEFINES) $(INCLUDES)
COPS+=-Wall -Werror -O2

CCPOPS=-fno-rtti

ifeq ($(shell uname -o),Cygwin)
	CCPOPS+=-std=gnu++11
else
	CCPOPS+=-std=c++11
endif	

SOURCE = ./src

CURR_DIR :=$(notdir $(patsubst %/,%,$(CURDIR)))
LIB_NAME :=$(patsubst lib-%,%,$(CURR_DIR))

BUILD = build_linux/

OBJECTS:=$(patsubst $(SOURCE)/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/*.c))
OBJECTS+=$(patsubst $(SOURCE)/%.cpp,$(BUILD)%.o,$(wildcard $(SOURCE)/*.cpp))
OBJECTS+=$(patsubst $(SOURCE)/linux/%.c,$(BUILD)linux/%.o,$(wildcard $(SOURCE)/linux/*.c))
OBJECTS+=$(patsubst $(SOURCE)/linux/%.cpp,$(BUILD)linux/%.o,$(wildcard $(SOURCE)/linux/*.cpp))

TARGET = lib_linux/lib$(LIB_NAME).a 

LIST = lib.list

all : builddirs $(TARGET)

.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD)linux 
	mkdir -p lib_linux

clean :
	rm -rf $(BUILD)
	rm -rf lib_linux	

$(BUILD)%.o: $(SOURCE)/%.c
	$(CC) $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)/%.cpp
	$(CPP) $(COPS) $(CCPOPS) $< -c -o $@
	
$(BUILD)linux/%.o: $(SOURCE)/linux/%.c
	$(CC) $(COPS) $< -c -o $@
	
$(BUILD)linux/%.o: $(SOURCE)/linux/%.cpp
	$(CPP) $(COPS) $(CCPOPS) $< -c -o $@	
	
$(TARGET): Makefile.Linux $(OBJECTS)
	$(AR) -r $(TARGET) $(OBJECTS)
	$(PREFIX)objdump -D $(TARGET) | $(PREFIX)c++filt > lib_linux/$(LIST)
