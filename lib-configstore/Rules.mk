$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONFIG_STORE_USE_FILE,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/file
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_I2C,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/i2c
	endif
	

	ifneq (,$(findstring CONFIG_STORE_USE_SPI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/spi
	endif
else
	EXTRA_SRCDIR+=device/file
	EXTRA_SRCDIR+=device/i2c
	EXTRA_SRCDIR+=device/spi
endif
