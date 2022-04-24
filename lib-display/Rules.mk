ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONFIG_DISPLAY_USE_SPI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=src/spi
	else
		EXTRA_SRCDIR+=src/i2c
	endif
else
	EXTRA_SRCDIR+=src/i2c
endif