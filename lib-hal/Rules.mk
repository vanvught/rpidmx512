$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONSOLE_I2C,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=console/i2c
	else
		ifneq (,$(findstring CONSOLE_FB,$(MAKE_FLAGS)))
			EXTRA_SRCDIR+=console console/h3
		else
			ifneq (,$(findstring CONSOLE_NULL,$(MAKE_FLAGS)))
				EXTRA_SRCDIR+=console/null
			else
				EXTRA_SRCDIR+=console/uart0
			endif
		endif
	endif
	ifneq ($(findstring DISABLE_RTC,$(MAKE_FLAGS)), DISABLE_RTC)
		EXTRA_SRCDIR+=rtc
		ifeq ($(findstring DISABLE_INTERNAL_RTC,$(MAKE_FLAGS)), DISABLE_INTERNAL_RTC)
			EXTRA_SRCDIR+=rtc/i2c
		else
			EXTRA_SRCDIR+=rtc/gd32
		endif
	endif
	ifneq (,$(findstring DEBUG_I2C,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=debug/i2c
		EXTRA_INCLUDES+=debug/i2c
	endif
	ifneq (,$(findstring DEBUG_STACK,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=debug/stack
	endif
else
	DEFINES+=DEBUG_I2C DEBUG_STACK
	EXTRA_INCLUDES+=debug/i2c
	EXTRA_SRCDIR+=console/i2c console/null	console/uart0	rtc debug/stack debug/i2c
endif
