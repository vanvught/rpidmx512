$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring DISABLE_INTERNAL_RTC,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/i2c
	endif
else
	EXTRA_SRCDIR+=src/i2c
endif
