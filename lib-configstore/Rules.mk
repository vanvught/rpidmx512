$(info $$MAKE_FLAGS [${MAKE_FLAGS}])

EXTRA_INCLUDES+=../lib-properties/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONFIG_STORE_USE_FILE,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/file
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_I2C,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/i2c
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_RAM,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/ram
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_ROM,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/rom
	endif
	
	ifneq (,$(findstring CONFIG_STORE_USE_SPI,$(MAKE_FLAGS)))
		EXTRA_SRCDIR+=device/spi
	endif
else
	EXTRA_SRCDIR+=device/file
	EXTRA_SRCDIR+=device/i2c
	EXTRA_SRCDIR+=device/ram
	EXTRA_SRCDIR+=device/rom
	EXTRA_SRCDIR+=device/spi
endif
