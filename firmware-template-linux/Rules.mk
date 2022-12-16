PREFIX ?=
DEF ?= 

CC	= $(PREFIX)gcc
CPP =$(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

$(info [${CURDIR}])

detected_OS := $(shell uname 2>/dev/null || echo Unknown)
detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))

$(info $$detected_OS [${detected_OS}])

ifeq ($(detected_OS),Darwin)
	LINUX=1
endif

ifeq ($(detected_OS),Linux) 
	LINUX=1
endif

include ../firmware-template/libs.mk

TTT=uuid
TMPVAR:=$(LIBS)
LIBS=$(filter-out $(TTT), $(TMPVAR))
LIBS+=debug

ifdef LINUX
	ifneq (, $(shell which /opt/vc/bin/vcgencmd))
		BCM2835 = ./../lib-bcm2835_raspbian
		ifneq "$(wildcard $(BCM2835) )" ""
			LIBS+=bcm2835_raspbian
		else
			LIBS+=bcm2835
		endif
		DEFINES+=RASPPI
	endif
endif

DEFINES:=$(addprefix -D,$(DEFINES))
DEFINES+=-DDISABLE_TFTP -DENABLE_HTTPD -DDISABLE_RTC
DEFINES+=-DCONFIG_STORE_USE_FILE

# The variable for the firmware include directories
INCDIRS=$(wildcard ./lib) $(wildcard ./include) $(wildcard ./*/include) ../firmware-template-linux/include
INCDIRS:=$(addprefix -I,$(INCDIRS))

# The variable for the libraries include directory
LIBINCDIRS=$(addprefix -I../lib-,$(LIBS))
LIBINCDIRS+=$(addprefix -I../lib-,$(DEFAULT_INCLUDES))
LIBINCDIRS:=$(addsuffix /include, $(LIBINCDIRS))

$(info $$LIBS [${LIBS}])

# The variables for the ld -L flag
LIB=$(addprefix -L../lib-,$(LIBS))
LIB:=$(addsuffix /lib_linux, $(LIB))

# The variable for the ld -l flag 
LDLIBS:=$(addprefix -l,$(LIBS))

# The variables for the dependency check 
LIBDEP=$(addprefix ../lib-,$(LIBS))

COPS=$(DEFINES) $(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS+=-g -Wall -Werror -Wextra -pedantic 
COPS+=-Wunused #-Wsign-conversion #-Wconversion

CCPOPS=-fno-rtti -fno-exceptions -fno-unwind-tables -Wnon-virtual-dtor

ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
else
	COPS+=-Wduplicated-cond -Wlogical-op #-Wduplicated-branches
#	CCPOPS+=-Wuseless-cast -Wold-style-cast
endif

ifeq ($(detected_OS),Cygwin)
	CCPOPS+=-std=gnu++11
else
	CCPOPS+=-std=c++11
endif

SOURCE = ./

CURR_DIR:=$(notdir $(patsubst %/,%,$(CURDIR)))

BUILD = build_linux/

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS+=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

BUILD_DIRS:=$(addprefix build_linux/,$(SRCDIR))

OBJECTS:=$(ASM_OBJECTS) $(C_OBJECTS)

TARGET = $(CURR_DIR)

define compile-objects
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) $(COPS) $(CCPOPS) -c $$< -o $$@	
endef

THISDIR = $(CURDIR)

all : builddirs prerequisites $(TARGET)
	
.PHONY: clean builddirs

builddirs:
	@mkdir -p $(BUILD_DIRS)
	[ -f generate_sofware_version_id.sh ] && chmod u+x generate_sofware_version_id.sh || true

clean: $(LIBDEP)
	rm -rf $(BUILD)
	rm -f $(TARGET)
	
#
# Libraries
#

.PHONY: libdep $(LIBDEP)

lisdep: $(LIBDEP)

$(LIBDEP):
	$(MAKE) -f Makefile.Linux $(MAKECMDGOALS) 'MAKE_FLAGS=$(DEFINES)' -C $@ 

# Build uImage

$(BUILD_DIRS) :
	mkdir -p $(BUILD_DIRS)
		
$(CURR_DIR) : Makefile $(LINKER) $(OBJECTS) $(LIBDEP)
	$(info $$TARGET [${TARGET}])
	$(CPP) $(OBJECTS) -o $(CURR_DIR) $(LIB) $(LDLIBS) -luuid -lpthread
	$(PREFIX)objdump -d $(TARGET) | $(PREFIX)c++filt > linux.lst

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
