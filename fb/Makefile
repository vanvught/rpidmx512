ARMGNU ?= arm-none-eabi

AOPS = --warn --fatal-warnings  -mcpu=arm1176jzf-s -march=armv6 -mfpu=vfp

COPS = -Wall -O3 -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mhard-float 
COPS += -DBARE_METAL
COPS += -I"./include"
COPS += -I"../bcm2835-baremetal/include"

LIB = -L /opt/gnuarm-hardfp/arm-none-eabi/lib/ -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.2 

BUILD = build/
SOURCE = ./
ASM_OBJECTS := $(patsubst $(SOURCE)/%.s,$(BUILD)%.o,$(wildcard $(SOURCE)/*.s))
C_OBJECTS := $(patsubst $(SOURCE)src/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)src/*.c)) $(patsubst $(SOURCE)firmware/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.c)) 
OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)

TARGET = lib/libfb.a

all : $(TARGET)

clean :
	rm -f $(BUILD)*.o
	rm -f $(TARGET)
	rm -f $(MAP)
	rm -f $(LIST)

$(BUILD)%.o: $(SOURCE)/%.S
	$(ARMGNU)-gcc $(COPS) -D__ASSEMBLY__ $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)firmware/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@				

$(BUILD)%.o: $(SOURCE)src/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@	

$(TARGET): Makefile $(OBJECTS)
	$(ARMGNU)-ar -r $(TARGET) $(OBJECTS) 
