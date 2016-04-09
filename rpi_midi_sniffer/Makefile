ARMGNU ?= arm-none-eabi

COPS_COMMON = -DBARE_METAL 
COPS_COMMON += -DENABLE_FRAMEBUFFER
COPS_COMMON += -DMIDI_SNIFFER
COPS_COMMON += -DMIDI_INTERFACE_UART
COPS_COMMON += -DMIDI_INTERFACE_SPI
COPS_COMMON += -I"./include"
COPS_COMMON += -I"./common/include"
COPS_COMMON += -I"../bcm2835-baremetal/include"
COPS_COMMON += -I"../bob-baremetal/include"
COPS_COMMON += -I"../fb/include"
COPS_COMMON += -I"../ff11/src"
COPS_COMMON += -I"../hal-baremetal/include"
COPS_COMMON += -I"../utils-lib/include"
COPS_COMMON += -Wall -Werror -O3 -nostartfiles -ffreestanding -mhard-float

COPS = -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=vfpv4 -mfloat-abi=hard -march=armv7-a -mtune=cortex-a7 # NEON is not enabled in vectors.s
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

LIB =  -L/opt/gnuarm-hardfp/arm-none-eabi/lib/armv6zk/arm1176jzf-s/hardfp/vfp
LIB += -L /opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv6zk/arm1176jzf-s/hardfp/vfp  
LIB += -L"../bob-baremetal/lib"
LIB += -L"../bcm2835-baremetal/lib"
LIB += -L"../emmc/lib"
LIB += -L"../fb/lib"
LIB += -L"../ff11/lib"
LIB += -L"../hal-baremetal/lib"
LIB += -L"../utils-lib/lib"

LIB7 =  -L/opt/gnuarm-hardfp/arm-none-eabi/lib/armv7-a/cortex-a7/hardfp/vfpv4
LIB7 += -L/opt/gnuarm-hardfp/lib/gcc/arm-none-eabi/4.9.3/armv7-a/cortex-a7/hardfp/vfpv4 
LIB7 += -L"../bob-baremetal/lib7"
LIB7 += -L"../bcm2835-baremetal/lib7"
LIB7 += -L"../emmc/lib7"
LIB7 += -L"../fb/lib7"
LIB7 += -L"../ff11/lib7"
LIB7 += -L"../hal-baremetal/lib7"
LIB7 += -L"../utils-lib/lib7"

SOURCE = ./

BUILD = build/
BUILD7 = build7/

C_OBJECTS := $(patsubst $(SOURCE)firmware/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/*.c)) $(patsubst $(SOURCE)firmware/lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)firmware/lib/*.c)) $(patsubst $(SOURCE)common/lib/%.c,$(BUILD)common/%.o,$(wildcard $(SOURCE)common/lib/*.c)) $(patsubst $(SOURCE)lib/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)lib/*.c)) $(patsubst $(SOURCE)devices/%.c,$(BUILD)devices/%.o,$(wildcard $(SOURCE)devices/*.c))
C_OBJECTS7 := $(patsubst $(SOURCE)firmware/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)firmware/*.c)) $(patsubst $(SOURCE)firmware/lib/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)firmware/lib/*.c)) $(patsubst $(SOURCE)common/lib/%.c,$(BUILD7)common/%.o,$(wildcard $(SOURCE)common/lib/*.c)) $(patsubst $(SOURCE)lib/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)lib/*.c)) $(patsubst $(SOURCE)devices/%.c,$(BUILD7)devices/%.o,$(wildcard $(SOURCE)devices/*.c))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)
OBJECTS7 := $(ASM_OBJECTS7) $(C_OBJECTS7)

TARGET = kernel.img
TARGET7 = kernel7.img

LIST = kernel.list
LIST7 = kernel7.list

MAP = kernel.map
MAP7 = kernel7.map

LINKER = firmware/memmap

all : builddirs $(TARGET) $(TARGET7)

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

# Build kernel.img

$(BUILD)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD)vectors.o

$(BUILD)common/%.o: $(SOURCE)common/lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@	
	
$(BUILD)%.o: $(SOURCE)firmware/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS) $< -c -o $@

$(BUILD)%.o: $(SOURCE)firmware/lib/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)lib/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@		
	
$(BUILD)main.elf : Makefile $(LINKER) $(BUILD)vectors.o $(OBJECTS) ../utils-lib/lib/libutils.a ../bob-baremetal/lib/libbob.a ../bcm2835-baremetal/lib/libbcm2835.a ../hal-baremetal/lib/libhal.a ../fb/lib/libfb.a ../emmc/lib/libemmc.a ../ff11/lib/libff11.a
	$(ARMGNU)-ld $(BUILD)vectors.o $(OBJECTS) -Map $(MAP) -T $(LINKER) -o $(BUILD)main.elf $(LIB) -lutils -lhal -lfb -lff11 -lemmc -lbob -lbcm2835 -lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD)main.elf > $(LIST)

$(TARGET) : $(BUILD)main.elf
	$(ARMGNU)-objcopy $(BUILD)main.elf -O binary $(TARGET)

# Build kernel7.img

$(BUILD7)vectors.o : firmware/vectors.S
	$(ARMGNU)-gcc $(COPS7) -D__ASSEMBLY__ -c firmware/vectors.S -o $(BUILD7)vectors.o

$(BUILD7)common/%.o: $(SOURCE)common/lib/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	
	
$(BUILD7)%.o: $(SOURCE)firmware/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS7) $< -c -o $@

$(BUILD7)%.o: $(SOURCE)firmware/lib/%.c
	$(ARMGNU)-gcc -std=c99 -pedantic $(COPS7) $< -c -o $@	
	
$(BUILD7)%.o: $(SOURCE)lib/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	
	
$(BUILD7)main.elf : Makefile $(LINKER) $(BUILD7)vectors.o $(OBJECTS7) ../utils-lib/lib7/libutils.a ../bob-baremetal/lib7/libbob.a ../bcm2835-baremetal/lib7/libbcm2835.a ../hal-baremetal/lib7/libhal.a ../fb/lib7/libfb.a ../emmc/lib7/libemmc.a ../ff11/lib7/libff11.a
	$(ARMGNU)-ld $(BUILD7)vectors.o $(OBJECTS7) -Map $(MAP7) -T $(LINKER) -o $(BUILD7)main.elf $(LIB7) -lutils -lhal -lfb -lff11 -lemmc -lbob -lbcm2835 -lc -lgcc
	$(ARMGNU)-objdump -D $(BUILD7)main.elf > $(LIST7)

$(TARGET7) : $(BUILD7)main.elf
	$(ARMGNU)-objcopy $(BUILD7)main.elf -O binary $(TARGET7)
