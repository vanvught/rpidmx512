PREFIX ?=
DEF ?= 

CC	= $(PREFIX)gcc
CPP =$(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

$(info [${CURDIR}])

ifeq ($(findstring ENABLE_SPIFLASH,$(DEFINES)),ENABLE_SPIFLASH)
	LIBS:=remoteconfig $(LIBS)
else
	LIBS:=$(LIBS)
endif

DEFINES:=$(addprefix -D,$(DEFINES))

detected_OS := $(shell uname 2>/dev/null || echo Unknown)
detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))

$(info $$detected_OS [${detected_OS}])

ifeq ($(detected_OS),Darwin) 
endif

ifeq ($(findstring ENABLE_SPIFLASH,$(DEFINES)),ENABLE_SPIFLASH)
	LIBS+=spiflashstore spiflashinstall spiflash
endif

LIBS+=network properties hal debug

ifeq ($(detected_OS),Linux) 
	ifneq (, $(shell which /opt/vc/bin/vcgencmd))
		BCM2835 = ./../lib-bcm2835_raspbian
		ifneq "$(wildcard $(BCM2835) )" ""
			LIBS+=bcm2835_raspbian
		else
			LIBS+=bcm2835
		endif
		DEFINES+=-DRASPPI
	endif
endif

ifeq ($(findstring displayudf,$(LIBS)),displayudf)
endif

# The variable for the firmware include directories
INCDIRS=$(wildcard ./lib) $(wildcard ./include) $(wildcard ./*/include)
INCDIRS:=$(addprefix -I,$(INCDIRS))

ifeq ($(findstring displayudf,$(LIBS)),displayudf)
	INCDIRS+=-I../lib-display/include
endif

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
LIBSDEP=$(addsuffix /lib_linux/lib, $(LIBDEP))
LIBSDEP:=$(join $(LIBSDEP), $(LIBS))
LIBSDEP:=$(addsuffix .a, $(LIBSDEP))

COPS=$(DEFINES) #-DNDEBUG
COPS+=$(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS+=-O2 -Wall -Werror -Wextra -pedantic -Wunused -Wsign-conversion #-Wconversion

CCPOPS=-fno-rtti -fno-exceptions -fno-unwind-tables -Wnon-virtual-dtor
ifeq ($(detected_OS),Darwin) 
else
CCPOPS+=-Wuseless-cast -Wold-style-cast
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

all : clearlibs builddirs prerequisites $(TARGET)
	
.PHONY: clean builddirs

clearlibs:
	$(MAKE) -f Makefile.Linux clean --directory=../lib-remoteconfig
	$(MAKE) -f Makefile.Linux clean --directory=../lib-spiflashstore
ifeq ($(findstring RDMNET_LLRP_ONLY,$(DEFINES)),RDMNET_LLRP_ONLY)
	$(MAKE) -f Makefile.Linux clean --directory=../lib-rdm
	$(MAKE) -f Makefile.Linux clean --directory=../lib-rdmsensor
	$(MAKE) -f Makefile.Linux clean --directory=../lib-rdmsubdevice
endif		

builddirs:
	@mkdir -p $(BUILD_DIRS)
	[ -f generate_sofware_version_id.sh ] && chmod u+x generate_sofware_version_id.sh || true

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
	for d in $(LIBDEP); \
		do                               \
			$(MAKE) -f Makefile.Linux clean --directory=$$d;       \
		done

$(LIBSDEP):
	for d in $(LIBDEP); \
		do                               \
			$(MAKE) -f Makefile.Linux 'MAKE_FLAGS=$(DEFINES)' --directory=$$d;       \
		done

$(BUILD_DIRS) :
	mkdir -p $(BUILD_DIRS)
		
$(CURR_DIR) : Makefile $(LINKER) $(OBJECTS) $(LIBSDEP)
	$(info $$TARGET [${TARGET}])
	$(CPP) $(OBJECTS) -o $(CURR_DIR) $(LIB) $(LDLIBS) -luuid
	$(PREFIX)objdump -d $(TARGET) | $(PREFIX)c++filt > linux.lst

$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
