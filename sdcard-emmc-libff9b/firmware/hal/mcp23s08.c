#include <bcm2835.h>
//
#include <mcp23s08.h>

void inline static mcp23s08_setup(device_info_t *device_info) {
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
	bcm2835_spi_setChipSelectPolarity(device_info->chip_select, LOW);
	bcm2835_spi_chipSelect(device_info->chip_select);
}

int mcp23s08_start(device_info_t *device_info) {

	if (bcm2835_init() != 1)
		return MCP23S08_ERROR;

	bcm2835_spi_begin();
	// Just once. Assuming all devices do have the same
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

	if (device_info->slave_address <= 0)
		device_info->slave_address = MCP23S08_DEFAULT_SLAVE_ADDRESS;

	return MCP23S08_OK;
}

void mcp23s08_end (void) {
	bcm2835_spi_end();
}

uint8_t mcp23s08_reg_read(device_info_t device_info, uint8_t reg) {
	char spiData[3];

	spiData[0] = MCP23S08_CMD_READ | ((device_info.slave_address & 7) << 1);
	spiData[1] = reg;

	mcp23s08_setup(&device_info);
	bcm2835_spi_transfern(spiData, 3);

	return spiData[2];
}

void mcp23s08_reg_write(device_info_t device_info, uint8_t reg, uint8_t value) {
	char spiData[3];

	spiData[0] = MCP23S08_CMD_WRITE | ((device_info.slave_address & 7) << 1);
	spiData[1] = reg;
	spiData[2] = value;

	mcp23s08_setup(&device_info);
	bcm2835_spi_transfern(spiData, 3);
}

void mcp23s08_gpio_fsel(device_info_t device_info, uint8_t pin, uint8_t mode) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_IODIR);
	if (mode == MCP23S08_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}
	mcp23s08_reg_write(device_info, MCP23S08_IODIR, data);
}

void mcp23s08_gpio_set(device_info_t device_info, uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_OLAT);
	data |= pin;
	mcp23s08_reg_write(device_info, MCP23S08_GPIO, data);
}

void mcp23s08_gpio_clr(device_info_t device_info, uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_OLAT);
	data &= (~pin);
	mcp23s08_reg_write(device_info, MCP23S08_GPIO, data);
}
