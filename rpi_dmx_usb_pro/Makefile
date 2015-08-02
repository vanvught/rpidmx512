ARMGNU ?= arm-none-eabi

AOPS = --warn --fatal-warnings  -mcpu=arm1176jzf-s -march=armv6 -mfpu=vfp

COPS_COMMON = -DBARE_METAL 
COPS_COMMON += -DENABLE_FRAMEBUFFER
COPS_COMMON += -DRDM_CONTROLLER
COPS_COMMON += -I"./include"
COPS_COMMON += -I"./common/include"
COPS_COMMON += -I"../bcm2835-baremetal/include"
COPS_COMMON += -I"../bob-baremetal/include"
COPS_COMMON += -I"../fb/include"
COPS_COMMON += -I"../ff9b/src"
COPS_COMMON += -I"../hal-baremetal/include"
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -ffreestanding -mhard-float
#COPS_COMMON += -I/opt/newlib/arm-none-eabi/include

COPS = -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s 
COPS += $(COPS_COMMON)

COPS7 = -mfpu=vfp -mfloat-abi=hard -march=armv7-a -mtune=cortex-a7 # NEON is not enabled in vectors.s
COPS7 += -DRPI2 -DRPIPLUS
COPS7 += $(COPS_COMMON)

LIB = -L/opt/gnuarm-hardfp/arm-none-eabi/lib/ -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.2 
LIB += -L"../bob-baremetal/lib"
LIB += -L"../bcm2835-baremetal/lib"
LIB += -L"../emmc/lib"
LIB += -L"../fb/lib"
LIB += -L"../ff9b/lib"
LIB += -L"../hal-baremetal/lib"
#LIB += -L/opt/newlib/arm-none-eabi/lib/fpu -L/usr/lib/gcc/arm-none-eabi/4.8/fpu/

LIB7 = -L /opt/gnuarm-hardfp/arm-none-eabi/lib/ -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.2 
LIB7 += -L"../bob-baremetal/lib7"
LIB7 += -L"../bcm2835-baremetal/lib7"
LIB7 += -L"../emmc/lib7"
LIB7 += -L"../fb/lib7"
LIB7 += -L"../ff9b/lib7"
LIB7 += -L"../hal-baremetal/lib7"
#LIB7 += -L/opt/newlib/arm-none-eabi/lib/fpu -L/usr/lib/gcc/arm-none-eabi/4.8/fpu/

SOURCE = ./

BUILD = build/
BUILD7 = build7/

ASM_OBJECTS := $(patsubst $(SOURCE)firmware/%.s,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.s))
ASM_OBJECTS7 := $(patsubst $(SOURCE)firmware/%.s,$(BUILD7)%.o,$(wildcard $(SOURCE)firmware/*.s))

C_OBJECTS := $(patsubst $(SOURCE)firmware/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.c)) $(patsubst $(SOURCE)firmware/lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/lib/*.c)) $(patsubst $(SOURCE)common/lib/%.c,$(BUILD)common/%.o,$(wildcard $(SOURCE)common/lib/*.c)) $(patsubst $(SOURCE)lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)lib/*.c))
C_OBJECTS7 := $(patsubst $(SOURCE)firmware/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)firmware/*.c)) $(patsubst $(SOURCE)firmware/lib/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)firmware/lib/*.c)) $(patsubst $(SOURCE)common/lib/%.c,$(BUILD7)common/%.o,$(wildcard $(SOURCE)common/lib/*.c)) $(patsubst $(SOURCE)lib/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)lib/*.c))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)
OBJECTS7 := $(ASM_OBJECTS7) $(C_OBJECTS7)

TARGET = kernel.img
TARGET7 = kernel7.img

LIST = kernel.list
LIST7 = kernel7.list

MAP = kernel.map
MAP7 = kernel7.map

LINKER = firmware/memmap

all : builddirs ./include/sofware_version_id.h $(TARGET) $(TARGET7)

.PHONY: clean builddirs

builddirs:
	@mkdir -p build/common build7/common

clean :
	rm -f $(BUILD)*.o
	rm -f $(BUILD7)*.o
	rm -f $(BUILD)common/*.o
	rm -f $(BUILD7)common/*.o
	rm -f $(TARGET)
	rm -f $(TARGET7)
	rm -f $(BUILD)*.elf
	rm -f $(BUILD7)*.elf
	rm -f $(MAP)
	rm -f $(MAP7)
	rm -f $(LIST)
	rm -f $(LIST7)
	rm -f ./include/sofware_version_id.h 
	
./include/sofware_version_id.h :
	sh ./generate_sofware_version_id.sh
	
# Build kernel.img

$(BUILD)vectors.o : firmware/vectors.s
	$(ARMGNU)-as $(AOPS) firmware/vectors.s -o $(BUILD)vectors.o

$(BUILD)common/%.o: $(SOURCE)common/lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@
	
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
	
# Build kernel7.img

$(BUILD7)vectors.o : firmware/vectors.s
	$(ARMGNU)-as $(AOPS) firmware/vectors.s -o $(BUILD7)vectors.o

$(BUILD7)common/%.o: $(SOURCE)common/lib/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@
	
$(BUILD7)%.o: $(SOURCE)firmware/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS7) $< -c -o $@

$(BUILD7)%.o: $(SOURCE)firmware/lib/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS7) $< -c -o $@	

$(BUILD7)%.o: $(SOURCE)lib/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	
	
$(BUILD7)main.elf : Makefile $(LINKER) $(OBJECTS7) ../bcm2835-baremetal/lib7/libbcm2835.a ../hal-baremetal/lib7/libhal.a ../bob-baremetal/lib7/libbob.a ../fb/lib7/libfb.a ../emmc/lib7/libemmc.a ../ff9b/lib7/libff9b.a 
	$(ARMGNU)-ld $(OBJECTS7) -Map $(MAP7) -T $(LINKER) -o $(BUILD7)main.elf $(LIB7) -lfb -lff9b -lemmc -lhal -lbob -lbcm2835 -lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD7)main.elf > $(LIST7)

$(TARGET7) : $(BUILD7)main.elf
	$(ARMGNU)-objcopy $(BUILD7)main.elf -O binary $(TARGET7)
