ARMGNU ?= arm-none-eabi

AOPS = --warn --fatal-warnings  -mcpu=arm1176jzf-s -march=armv6 -mfpu=vfp

COPS = -Wall -O3 -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mhard-float 
COPS += -DBARE_METAL
COPS += -DENABLE_FRAMEBUFFER
#COPS += -DBW_I2C_UI
COPS += -I"./include"
COPS += -I"./firmware/include"
#COPS += -I"../ff9b/src"
COPS += -I"../fb" -I"../fb/include"
#COPS += -I"../hal/include"

LIB = -L /opt/gnuarm-hardfp/arm-none-eabi/lib/ -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.7.3 
LIB += -L"../fb/lib"
#LIB += -L"../hal/lib"
#LIB += -L"../emmc/lib"
#LIB += -L"../ff9b/lib"

BUILD = build/
SOURCE = ./
ASM_OBJECTS := $(patsubst $(SOURCE)firmware/%.s,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.s)) $(patsubst $(SOURCE)firmware/lib/%.S,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/lib/*.S))
C_OBJECTS := $(patsubst $(SOURCE)common/lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)common/lib/*.c)) $(patsubst $(SOURCE)lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)lib/*.c)) $(patsubst $(SOURCE)firmware/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.c)) $(patsubst $(SOURCE)firmware/lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/lib/*.c)) $(patsubst $(SOURCE)firmware/hal/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/hal/*.c)) $(patsubst $(SOURCE)devices/%.c,$(BUILD)devices/%.o,$(wildcard $(SOURCE)devices/*.c))
OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)

TARGET = kernel.img
LINKER = firmware/memmap
MAP = kernel.map
LIST = kernel.list

all : $(TARGET)

clean :
	rm -f $(BUILD)*.o
	rm -f $(TARGET)
	rm -f $(BUILD)*.elf
	rm -f $(MAP)
	rm -f $(LIST)

$(BUILD)vectors.o : firmware/vectors.s
	$(ARMGNU)-as $(AOPS) firmware/vectors.s -o $(BUILD)vectors.o
	
$(BUILD)%.o: $(SOURCE)firmware/lib/%.S
	$(ARMGNU)-gcc $(COPS) -D__ASSEMBLY__ $< -c -o $@		

$(BUILD)%.o: $(SOURCE)common/lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@		

$(BUILD)%.o: $(SOURCE)firmware/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@

$(BUILD)%.o: $(SOURCE)firmware/hal/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)firmware/lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@	
	
$(BUILD)%.o: $(SOURCE)lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@	
	
$(BUILD)main.elf : Makefile $(LINKER) $(OBJECTS) ../fb/lib/libfb.a
	$(ARMGNU)-ld $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIB) -lfb -lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD)main.elf > $(LIST)

$(TARGET) : $(BUILD)main.elf
	$(ARMGNU)-objcopy $(BUILD)main.elf -O binary $(TARGET)