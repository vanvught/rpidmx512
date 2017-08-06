PREFIX ?= 

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(CC)
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

LIBS +=  network properties

DEFINES := $(addprefix -D,$(DEFINES))

# The variable for the firmware include directories
INCDIRS = $(wildcard ./include) $(wildcard ./*/include)
INCDIRS := $(addprefix -I,$(INCDIRS))

# The variable for the libraries include directory
LIBINCDIRS = $(addprefix -I../lib-,$(LIBS))
LIBINCDIRS := $(addsuffix /include, $(LIBINCDIRS))

# The variables for the ld -L flag
LIB  = $(addprefix -L../lib-,$(LIBS))
LIB := $(addsuffix /lib_linux, $(LIB))

# The variable for the ld -l flag 
LDLIBS := $(addprefix -l,$(LIBS))

# The variables for the dependency check 
LIBDEP = $(addprefix ../lib-,$(LIBS))
LIBDEP := $(addsuffix /lib_linux/lib, $(LIBDEP))
LIBDEP := $(join $(LIB6DEP), $(LIBS))
LIBDEP := $(addsuffix .a, $(LIB6DEP))

COPS = $(DEFINES) #-DNDEBUG
COPS += $(INCDIRS) $(LIBINCDIRS) $(addprefix -I,$(EXTRA_INCLUDES))
COPS += -Wall -Werror -O3

SOURCE = ./

CURR_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))

BUILD = build_linux/

C_OBJECTS = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
C_OBJECTS7 = $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
C_OBJECTS7 += $(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD7)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))

BUILD_DIRS := $(addprefix build_linux/,$(SRCDIR))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)

TARGET = $(CURR_DIR)

define compile-objects
$(BUILD)$1/%.o: $(SOURCE)$1/%.c
	$(CC) $(COPS) -c $$< -o $$@
	
$(BUILD)$1/%.o: $(SOURCE)$1/%.cpp
	$(CPP) -pedantic -fno-exceptions -fno-unwind-tables -fno-rtti -std=c++11 $(COPS) -c $$< -o $$@	
endef

THISDIR = $(CURDIR)

all : builddirs prerequisites $(TARGET) $(TARGET7)
	
.PHONY: clean builddirs

buildlibs:
	cd .. && ./makeall-lib.sh && cd $(THISDIR)

builddirs:
	@mkdir -p $(BUILD_DIRS)

clean:
	rm -rf $(BUILD)
	rm $(TARGET)

$(CURR_DIR) : Makefile $(LINKER) $(OBJECTS) $(LIBDEP)
	$(CPP) $(OBJECTS) -o $(CURR_DIR) $(LIB) $(LDLIBS)


$(foreach bdir,$(SRCDIR),$(eval $(call compile-objects,$(bdir))))
