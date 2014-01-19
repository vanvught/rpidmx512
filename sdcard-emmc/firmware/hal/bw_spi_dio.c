#include <bcm2835.h>
//
#include <bw_dio.h>

#ifndef BARE_METAL
static void uwait(int us) { bcm2835_delayMicroseconds(us); }
#else
extern void uwait(int);
#endif

extern int printf(const char *format, ...);

static char spi_dio_slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;

inline void static dio_spi_setup(void) {
	bcm2835_spi_setClockDivider(2500); // 100kHz
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
}

int bw_spi_dio_start (char slave_address) {

	if (bcm2835_init() != 1)
		return 1;

	bcm2835_spi_begin();
	// Just once. Assuming all devices do have the same
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

	if (slave_address <= 0)
		spi_dio_slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;
	else
		spi_dio_slave_address = slave_address;

	return 0;
}

void bw_spi_dio_end(void) {
	bcm2835_spi_end();
	bcm2835_close();
}

void bw_spi_dio_fsel_mask(unsigned char mask) {
	char cmd[3];

	cmd[0] = spi_dio_slave_address;
	cmd[1] = BW_PORT_WRITE_IO_DIRECTION;
	cmd[2] = mask;
	dio_spi_setup();

	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_DIO_SPI_BYTE_WAIT_US);
}

void bw_spi_dio_output(unsigned char pins) {
	char cmd[3];

	cmd[0] = spi_dio_slave_address;
	cmd[1] = BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[2] = pins;

	dio_spi_setup();
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_DIO_SPI_BYTE_WAIT_US);
}

void bw_spi_dio_read_id(void) {
	char buf[BW_DIO_ID_STRING_LENGTH + 1];
	int i = 0;
	for (i = 0; i <= BW_DIO_ID_STRING_LENGTH; i++) {
		buf[i] = '\0';
	}

	buf[0] = spi_dio_slave_address | 1;
	buf[1] = BW_PORT_READ_ID_STRING;

	dio_spi_setup();
	bcm2835_spi_transfern(buf, BW_DIO_ID_STRING_LENGTH);
	uwait(BW_DIO_SPI_BYTE_WAIT_US);
	printf("[%s]\n", &buf[1]);
}
