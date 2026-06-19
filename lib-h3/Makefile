DEFINES=NDEBUG
DEFINES+=CMSIS_device_header=\"h3.h\"

EXTRA_SRCDIR=CMSIS/Core_A/Source arm device/mmc firmware

EXTRA_INCLUDES=CMSIS/Core_A/Include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONSOLE_FB,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/fb
		EXTRA_SRCDIR+=console console/fb
	else
		ifneq (,$(findstring CONSOLE_NULL,$(MAKE_FLAGS)))
			EXTRA_SRCDIR+=console/null
		else
			EXTRA_SRCDIR+=console/uart0
		endif
	endif
		
	ifneq (,$(findstring NO_EMAC,$(NO_EMAC)))
	else
		EXTRA_SRCDIR+=device/emac
	endif
	
	ifeq (,$(findstring DISABLE_FS,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-fatfs/ff14b
	endif

else
	EXTRA_SRCDIR+=device/fb device/emac
	EXTRA_SRCDIR+=console console/h3
endif

include ../firmware-template-h3/lib/Rules.mk
