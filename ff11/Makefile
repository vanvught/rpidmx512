ARMGNU ?= arm-none-eabi

COPS_COMMON = -std=c99 -pedantic -Wall -O3 -nostartfiles -ffreestanding -mhard-float  

COPS = -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s 
COPS += $(COPS_COMMON)

COPS7 = -mfpu=vfpv4 -mfloat-abi=hard -march=armv7-a -mtune=cortex-a7 # NEON is not enabled in vectors.s
COPS7 += -DRPI2 -DRPIPLUS
COPS7 += $(COPS_COMMON)

SOURCE = ./src

BUILD = build/
BUILD7 = build7/

OBJECTS := $(patsubst $(SOURCE)/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/*.c)) $(patsubst $(SOURCE)/option/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/option/*.c))
OBJECTS7 := $(patsubst $(SOURCE)/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)/*.c)) $(patsubst $(SOURCE)/option/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/option/*.c))

TARGET = lib/libff11.a 
TARGET7 = lib7/libff11.a

LIST = lib.list
LIST7 = lib7.list

all : builddirs $(TARGET) $(TARGET7)

.PHONY: clean builddirs

builddirs: 
	@mkdir -p build build7 lib lib7

clean :
	rm -f $(BUILD)*.o
	rm -f $(BUILD7)*.o
	rm -f $(TARGET)
	rm -f $(TARGET7)
	rm -f $(LIST)
	rm -f $(LIST7)

$(BUILD)%.o: $(SOURCE)/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)/option/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@			

$(TARGET): Makefile $(OBJECTS)
	$(ARMGNU)-ar -r $(TARGET) $(OBJECTS) 
	$(ARMGNU)-objdump -D $(TARGET) > $(LIST)
	
$(BUILD7)%.o: $(SOURCE)/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	
	
$(BUILD7)%.o: $(SOURCE)/option/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	

$(TARGET7): Makefile $(OBJECTS7)
	$(ARMGNU)-ar -r $(TARGET7) $(OBJECTS7) 
	$(ARMGNU)-objdump -D $(TARGET7) > $(LIST7)
