ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONFIG_DISPLAY_USE_SPI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/spi src/sleep
	else
		ifneq (,$(findstring CONFIG_DISPLAY_USE_CUSTOM,$(MAKE_FLAGS)))
		else
			EXTRA_SRCDIR+=src/i2c src/sleep
		endif
	endif
else
	DEFINES+=CONFIG_DISPLAY_ENABLE_CURSOR_MODE
	DEFINES+=CONFIG_DISPLAY_FIX_FLIP_VERTICALLY
	EXTRA_SRCDIR+=src/i2c src/sleep
endif
