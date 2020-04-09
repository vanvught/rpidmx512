PREFIX ?=

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

ROOT= ./../..
SOURCE= .
BUILD=build_linux/

LIBS=rdmnet rdm rdmsensor rdmsubdevice e131 lightset network properties debug hal

# The variable for the libraries include directory
LIBINCDIRS=$(addprefix -I$(ROOT)/lib-,$(LIBS))
LIBINCDIRS:=$(addsuffix /include, $(LIBINCDIRS))
# The variables for the ld -L flag
LIB=$(addprefix -L$(ROOT)/lib-,$(LIBS))
LIB:=$(addsuffix /lib_linux, $(LIB))
# The variable for the ld -l flag 
LDLIBS:=$(addprefix -l,$(LIBS))
# The variables for the dependency check 
LIBDEP=$(addprefix $(ROOT)/lib-,$(LIBS))
LIBSDEP=$(addsuffix /lib_linux/lib, $(LIBDEP))
LIBSDEP:=$(join $(LIBSDEP), $(LIBS))
LIBSDEP:=$(addsuffix .a, $(LIBSDEP))

COPS=$(DEFINES) $(LIBINCDIRS) -Wall -Werror -O2 -fno-rtti -std=c++11 -DNDEBUG
OBJECTS=$(foreach sdir,$(SOURCE),$(patsubst $(sdir)/%.cpp,$(BUILD)%.o,$(wildcard $(sdir)/*.cpp)))
BUILD_DIRS=build_linux
TARGET=dummy_device

define compile-objects
$(BUILD)%.o: %.cpp
	$(CPP) $(COPS) -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti -std=c++11 -c $$< -o $$@	
endef

all : prerequisites builddirs $(TARGET)
	
.PHONY: clean builddirs

builddirs:
	@mkdir -p $(BUILD_DIRS)

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
	for d in $(LIBDEP); \
	do                               \
		$(MAKE) -f Makefile.Linux clean --directory=$$d;       \
	done
	
prerequisites:
	./generate_sofware_version_id.sh

$(LIBSDEP):
	for d in $(LIBDEP); \
		do                               \
			$(MAKE) -f Makefile.Linux 'DEFINES=-DDMX_WORKSHOP_DEFECT -DRDMNET_LLRP_ONLY -DNDEBUG' --directory=$$d;       \
		done

$(TARGET) : Makefile $(LINKER) $(OBJECTS) $(LIBDEP) $(LIBSDEP)
	$(CPP) $(OBJECTS) -o $(TARGET) $(LIB) $(LDLIBS) -luuid

$(foreach bdir,$(SOURCE),$(eval $(call compile-objects)))
