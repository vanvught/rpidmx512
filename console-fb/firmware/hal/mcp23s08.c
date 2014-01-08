#include <bcm2835.h>
#include <mcp23s08.h>

char spi_mcp23s08_slave_address = MCP23S08_DEFAULT_SLAVE_ADDRESS;

void inline static mcp23s08_setup(void) {
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
}

int mcp23s08_start (char slave_address) {

	if (bcm2835_init() != 1)
		return MCP23S08_ERROR;

	bcm2835_spi_begin();

	if (slave_address <= 0)
		spi_mcp23s08_slave_address = MCP23S08_DEFAULT_SLAVE_ADDRESS;
	else
		spi_mcp23s08_slave_address = slave_address;

	return MCP23S08_OK;
}

void mcp23s08_end (void) {
	bcm2835_spi_end();
}

uint8_t mcp23s08_reg_read(uint8_t reg) {
	char spiData[3];

	spiData[0] = MCP23S08_CMD_READ | ((spi_mcp23s08_slave_address & 7) << 1);
	spiData[1] = reg;

	mcp23s08_setup();
	bcm2835_spi_transfern(spiData, 3);

	return spiData[2];
}

void mcp23s08_reg_write(uint8_t reg, uint8_t value) {
	char spiData[3];

	spiData[0] = MCP23S08_CMD_WRITE | ((spi_mcp23s08_slave_address & 7) << 1);
	spiData[1] = reg;
	spiData[2] = value;

	mcp23s08_setup();
	bcm2835_spi_transfern(spiData, 3);
}

void mcp23s08_gpio_fsel(uint8_t pin, uint8_t mode) {
	uint8_t data = mcp23s08_reg_read(MCP23S08_IODIR);
	if (mode == MCP23S08_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}
	mcp23s08_reg_write(MCP23S08_IODIR, data);
}

void mcp23s08_gpio_set(uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(MCP23S08_OLAT);
	data |= pin;
	mcp23s08_reg_write(MCP23S08_GPIO, data);
}

void mcp23s08_gpio_clr(uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(MCP23S08_OLAT);
	data &= (~pin);
	mcp23s08_reg_write(MCP23S08_GPIO, data);
}
