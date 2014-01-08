#include <bcm2835.h>
#include <bw.h>
#include <bw_lcd.h>

#ifndef BARE_METAL
static void uwait(int us) { bcm2835_delayMicroseconds(us); }
#else
extern void uwait(int);
#endif

extern int printf(const char *format, ...);

char spi_lcd_slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;

void inline static lcd_spi_setup(void) {
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
}

int bw_spi_lcd_start (char slave_address) {

	if (bcm2835_init() != 1)
		return BW_LCD_ERROR;

	bcm2835_spi_begin();

	if (slave_address <= 0)
		spi_lcd_slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	else
		spi_lcd_slave_address = slave_address;

	return BW_LCD_OK;
}

void bw_spi_lcd_end(void) {
	bcm2835_spi_end();
	bcm2835_close();
}

void bw_spi_lcd_set_cursor(uint8_t line, uint8_t pos) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0x00 };
	cmd[0] = spi_lcd_slave_address;
	if (line > BW_LCD_MAX_LINES)
		line = 0;
	if (pos > BW_LCD_MAX_CHARACTERS)
		pos = 0;
	cmd[2] = ((line && 0b11) << 5) | (pos && 0b11111);
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
}

void bw_spi_lcd_text(const char *text, uint8_t length) {
	static char data[22];
	data[0] = spi_lcd_slave_address;
	data[1] = BW_PORT_WRITE_DISPLAY_DATA;
	if (length > BW_LCD_MAX_CHARACTERS)
		length = BW_LCD_MAX_CHARACTERS;
	uint8_t i;
	for (i = 0; i < length; i++)
		data[i + 2] = text[i];
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(data, length + 2);
}

void bw_spi_lcd_text_line_1(const char *text, uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b0000000 };
	cmd[0] = spi_lcd_slave_address;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	bw_spi_lcd_text(text, length);
}

void bw_spi_lcd_text_line_2(const char *text, uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b0100000 };
	cmd[0] = spi_lcd_slave_address;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	bw_spi_lcd_text(text, length);
}

void bw_spi_lcd_text_line_3(const char *text, uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b1000000 };
	cmd[0] = spi_lcd_slave_address;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	bw_spi_lcd_text(text, length);
}

void bw_spi_lcd_text_line_4(const char *text, uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b1100000 };
	cmd[0] = spi_lcd_slave_address;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	bw_spi_lcd_text(text, length);
}

void bw_spi_lcd_cls(void) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_CLEAR_SCREEN, ' ' };
	cmd[0] = spi_lcd_slave_address;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
}

void bw_spi_lcd_set_contrast(uint8_t value) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[0] = spi_lcd_slave_address;
	cmd[2] = value;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
}

void bw_spi_lcd_set_backlight(uint8_t value) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[0] = spi_lcd_slave_address;
	cmd[2] = value;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
}

void bw_spi_lcd_reinit(void) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_REINIT_LCD, ' ' };
	cmd[0] = spi_lcd_slave_address;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(500000);
}

void bw_spi_lcd_read_id(void) {
	char buf[BW_LCD_ID_STRING_LENGTH + 1] = { 0x00, BW_PORT_READ_ID_STRING };
	buf[0] = spi_lcd_slave_address + 1;
	lcd_spi_setup();
	uwait(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_transfern(buf, BW_LCD_ID_STRING_LENGTH);
	buf[BW_LCD_ID_STRING_LENGTH] = '\0';
	printf("[%s]\r\n", buf);
}
