$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_SRCDIR=console/uart0

ifneq ($(MAKE_FLAGS),)
	ifneq ($(findstring NDEBUG,$(MAKE_FLAGS)), NDEBUG)
		EXTRA_SRCDIR+=debug
	endif
	ifneq ($(findstring DISABLE_RTC,$(MAKE_FLAGS)), DISABLE_RTC)
		EXTRA_SRCDIR+=rtc
		ifeq ($(findstring DISABLE_INTERNAL_RTC,$(MAKE_FLAGS)), DISABLE_INTERNAL_RTC)
			EXTRA_SRCDIR+=rtc/i2c
		else
			EXTRA_SRCDIR+=rtc/gd32
		endif
	endif
else
	EXTRA_SRCDIR+=rtc debug
endif
