ARMGNU ?= arm-none-eabi

COPS_COMMON = -std=c99 -pedantic -Wall -O2 -nostdlib -nostartfiles -ffreestanding -mhard-float -mfloat-abi=hard

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

SOURCE = ./src

BUILD = build/
BUILD7 = build7/

OBJECTS := $(patsubst $(SOURCE)/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/*.c)) $(patsubst $(SOURCE)/option/%.c,$(BUILD)%.o,$(wildcard $(SOURCE)/option/*.c))
OBJECTS7 := $(patsubst $(SOURCE)/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)/*.c)) $(patsubst $(SOURCE)/option/%.c,$(BUILD7)%.o,$(wildcard $(SOURCE)/option/*.c))

TARGET = lib/libff12c.a 
TARGET7 = lib7/libff12c.a

LIST = lib.list
LIST7 = lib7.list

all : builddirs $(TARGET) $(TARGET7)

.PHONY: clean builddirs

builddirs: 
	@mkdir -p $(BUILD) $(BUILD7) lib lib7 
	
clean :
	rm -rf $(BUILD) $(BUILD7)
	rm -rf lib/ lib7/
	rm -f $(LIST)
	rm -f $(LIST7)
	
# ARM v6

$(BUILD)%.o: $(SOURCE)/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@
	
$(BUILD)%.o: $(SOURCE)/option/%.c
	$(ARMGNU)-gcc $(COPS) $< -c -o $@			

$(TARGET): Makefile $(OBJECTS)
	$(ARMGNU)-ar -r $(TARGET) $(OBJECTS) 
	$(ARMGNU)-objdump -D $(TARGET) > $(LIST)
	
# ARM v7	
	
$(BUILD7)%.o: $(SOURCE)/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	
	
$(BUILD7)%.o: $(SOURCE)/option/%.c
	$(ARMGNU)-gcc $(COPS7) $< -c -o $@	

$(TARGET7): Makefile $(OBJECTS7)
	$(ARMGNU)-ar -r $(TARGET7) $(OBJECTS7) 
	$(ARMGNU)-objdump -D $(TARGET7) > $(LIST7)
