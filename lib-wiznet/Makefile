PREFIX ?= arm-none-eabi-

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

COPS_COMMON =-DBARE_METAL -DNDEBUG
COPS_COMMON +=-I./include -I./ioLibrary_Driver/Ethernet -I./../lib-bcm2835/include -I./../lib-debug/include -I./../include
#COPS_COMMON +=-Wall -Werror 
COPS_COMMON +=-O2 -nostartfiles -nostdinc -nostdlib -ffreestanding -mhard-float -mfloat-abi=hard

COPS = -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s
COPS += -DRPI1
COPS += $(COPS_COMMON)

COPS7 = -mfpu=neon-vfpv4 -march=armv7-a -mtune=cortex-a7
COPS7 += -DRPI2
COPS7 += $(COPS_COMMON)

SRCDIR = ioLibrary_Driver/Ethernet ioLibrary_Driver/Internet/DHCP src

SRCDIRW5100 = $(SRCDIR) ioLibrary_Driver/Ethernet/W5100 src/W5100
SRCDIRW5500 = $(SRCDIR) ioLibrary_Driver/Ethernet/W5500 src/W5500

TARGETW5100 = lib/libwiznet5100.a
TARGETW5500 = lib/libwiznet5500.a
 
TARGET7W5100 = lib7/libwiznet5100.a
TARGET7W5500 = lib7/libwiznet5500.a

TARGETS = $(TARGETW5100) $(TARGET7W5100) $(TARGETW5500) $(TARGET7W5500)

LISTW5100 = libwiznet5100.list
LISTW5500 = libwiznet5500.list

LIST7W5100 = libwiznet5100.list
LIST7W5500 = libwiznet5500.list

BUILDW5100 = build/W5100/
BUILDW5500 = build/W5500/

BUILD7W5100 = build7/W5100/
BUILD7W5500 = build7/W5500/

OBJECTSW5100 = $(foreach sdir,$(SRCDIRW5100),$(patsubst $(sdir)/%.c,$(BUILDW5100)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
OBJECTSW5500 = $(foreach sdir,$(SRCDIRW5500),$(patsubst $(sdir)/%.c,$(BUILDW5500)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))

OBJECTS7W5100 = $(foreach sdir,$(SRCDIRW5100),$(patsubst $(sdir)/%.c,$(BUILD7W5100)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
OBJECTS7W5500 = $(foreach sdir,$(SRCDIRW5500),$(patsubst $(sdir)/%.c,$(BUILD7W5500)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))

BUILD_DIRS := $(addprefix $(BUILDW5100),$(SRCDIRW5100)) $(addprefix $(BUILDW5500),$(SRCDIRW5500))
BUILD7_DIRS := $(addprefix $(BUILD7W5100),$(SRCDIRW5100)) $(addprefix $(BUILD7W5500),$(SRCDIRW5500))

define compile-objects6-W5100
$(BUILDW5100)$1/%.o: $(SOURCEW5100)$1/%.c
	$(CC) $(COPS) -D_WIZCHIP_=5100 -c $$< -o $$@
endef

define compile-objects7-W5100
$(BUILD7W5100)$1/%.o: $(SOURCEW5100)$1/%.c
	$(CC) $(COPS7) -D_WIZCHIP_=5100 -c $$< -o $$@
endef

define compile-objects6-W5500
$(BUILDW5500)$1/%.o: $(SOURCEW5500)$1/%.c
	$(CC) $(COPS) -D_WIZCHIP_=5500 -c $$< -o $$@
endef

define compile-objects7-W5500
$(BUILD7W5500)$1/%.o: $(SOURCEW5500)$1/%.c
	$(CC) $(COPS7) -D_WIZCHIP_=5500 -c $$< -o $$@
endef

all : builddirs $(TARGETS)

.PHONY: clean builddirs

builddirs:
	mkdir -p $(BUILD_DIRS) $(BUILD7_DIRS)
	mkdir -p lib lib7

clean :
	rm -rf build/ build7/
	rm -rf lib/ lib7/

# Build lib

$(TARGETW5100): Makefile $(OBJECTSW5100)
	$(AR) -r $(TARGETW5100) $(OBJECTSW5100) 
	$(PREFIX)objdump -D $(TARGETW5100) > lib/$(LISTW5100)

$(TARGETW5500): Makefile $(OBJECTSW5500)
	$(AR) -r $(TARGETW5500) $(OBJECTSW5500) 
	$(PREFIX)objdump -D $(TARGETW5500) > lib/$(LISTW5500)

# Build lib7

$(TARGET7W5100): Makefile $(OBJECTS7W5100)
	$(AR) -r $(TARGET7W5100) $(OBJECTS7W5100) 
	$(PREFIX)objdump -D $(TARGET7W5100) > lib7/$(LIST7W5100)

$(TARGET7W5500): Makefile $(OBJECTS7W5500)
	$(AR) -r $(TARGET7W5500) $(OBJECTS7W5500) 
	$(PREFIX)objdump -D $(TARGET7W5500) > lib7/$(LIST7W5500)
	
$(foreach bdir,$(SRCDIRW5100),$(eval $(call compile-objects6-W5100,$(bdir))))
$(foreach bdir,$(SRCDIRW5500),$(eval $(call compile-objects6-W5500,$(bdir))))

$(foreach bdir,$(SRCDIRW5100),$(eval $(call compile-objects7-W5100,$(bdir))))
$(foreach bdir,$(SRCDIRW5500),$(eval $(call compile-objects7-W5500,$(bdir))))
