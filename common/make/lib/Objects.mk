$(info "lib/Objects.mk")

C_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.c,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.c)))
CPP_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.cpp,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.cpp)))
ASM_OBJECTS=$(foreach sdir,$(SRCDIR),$(patsubst $(sdir)/%.S,$(BUILD)$(sdir)/%.o,$(wildcard $(sdir)/*.S)))

ifneq ($(EXTRA_C_SOURCE_FILES),)
	EXTRA_C_OBJECTS := $(patsubst %.c,$(BUILD)%.o,$(EXTRA_C_SOURCE_FILES))
	EXTRA_C_DIRECTORIES := $(sort $(shell dirname $(EXTRA_C_SOURCE_FILES)))
	EXTRA_C_BUILD_DIRS  := $(addprefix $(BUILD),$(EXTRA_C_DIRECTORIES))
endif

ifneq ($(EXTRA_CPP_SOURCE_FILES),)
	EXTRA_CPP_OBJECTS := $(patsubst %.cpp,$(BUILD)%.o,$(EXTRA_CPP_SOURCE_FILES))
	EXTRA_CPP_DIRECTORIES := $(sort $(shell dirname $(EXTRA_CPP_SOURCE_FILES)))
	EXTRA_CPP_BUILD_DIRS  := $(addprefix $(BUILD),$(EXTRA_CPP_DIRECTORIES))
endif

OBJECTS:=$(strip $(ASM_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS) $(EXTRA_C_OBJECTS) $(EXTRA_CPP_OBJECTS))
