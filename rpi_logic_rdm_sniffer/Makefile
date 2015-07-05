ARMGNU ?= arm-none-eabi

AOPS = --warn --fatal-warnings  -mcpu=arm1176jzf-s -march=armv6 -mfpu=vfp

COPS = -Wall -Werror -O3 -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mhard-float 
COPS += -DBARE_METAL
COPS += -DENABLE_FRAMEBUFFER
#COPS += -DBW_I2C_UI
#COPS += -DDEBUG
COPS += -DLOGIC_ANALYZER
COPS += -DRDM_CONTROLLER

COPS += -I"./common/include"
COPS += -I"./include"
COPS += -I"../ff9b/src"
COPS += -I"../fb/include"
COPS += -I"../bob-baremetal/include"
COPS += -I"../bcm2835-baremetal/include"
COPS += -I"../hal-baremetal/include"

LIB = -L /opt/gnuarm-hardfp/arm-none-eabi/lib/ -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.2 
LIB += -L"../fb/lib"
LIB += -L"../bob-baremetal/lib"
LIB += -L"../bcm2835-baremetal/lib"
LIB += -L"../hal-baremetal/lib"
LIB += -L"../emmc/lib"
LIB += -L"../ff9b/lib"

BUILD = build/
SOURCE = ./
ASM_OBJECTS := $(patsubst $(SOURCE)firmware/%.s,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.s)) $(patsubst $(SOURCE)firmware/lib/%.S,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/lib/*.S))
C_OBJECTS := $(patsubst $(SOURCE)common/lib/%.c,$(BUILD)common/%.o,$(wildcard $(SOURCE)common/lib/*.c)) $(patsubst $(SOURCE)lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)lib/*.c)) $(patsubst $(SOURCE)firmware/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.c)) $(patsubst $(SOURCE)firmware/lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/lib/*.c)) $(patsubst $(SOURCE)devices/%.c,$(BUILD)devices/%.o,$(wildcard $(SOURCE)devices/*.c))
OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)

TARGET = kernel.img
LINKER = firmware/memmap
MAP = kernel.map
LIST = kernel.list

all : $(TARGET)

clean :
	rm -f $(BUILD)*.o
	rm -f $(BUILD)common/*.o
#	rm -f $(BUILD)devices/*.o	
	rm -f $(TARGET)
	rm -f $(BUILD)*.elf
	rm -f $(MAP)
	rm -f $(LIST)
#	rm -f ./include/sofware_version_id.h 

#./include/sofware_version_id.h :
#	sh ./generate_sofware_version_id.sh
	
$(BUILD)common/%.o: $(SOURCE)common/lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@
	
$(BUILD)devices/%.o: $(SOURCE)devices/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@		

$(BUILD)vectors.o : firmware/vectors.s
	$(ARMGNU)-as $(AOPS) firmware/vectors.s -o $(BUILD)vectors.o
	
$(BUILD)%.o: $(SOURCE)firmware/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)firmware/lib/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS) $< -c -o $@		

$(BUILD)%.o: $(SOURCE)lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@	
	
$(BUILD)main.elf : Makefile $(LINKER) $(OBJECTS) ../bcm2835-baremetal/lib/libbcm2835.a ../hal-baremetal/lib/libhal.a ../bob-baremetal/lib/libbob.a ../fb/lib/libfb.a ../emmc/lib/libemmc.a ../ff9b/lib/libff9b.a 
	$(ARMGNU)-ld $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIB) -lfb -lff9b -lemmc -lhal -lbob -lbcm2835 -lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD)main.elf > $(LIST)

$(TARGET) : $(BUILD)main.elf
	$(ARMGNU)-objcopy $(BUILD)main.elf -O binary $(TARGET)