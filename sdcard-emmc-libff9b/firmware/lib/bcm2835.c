#include <bcm2835.h>

#define BCM2835_PERI_SET_BITS(a, v, m)		a = ((a) & ~(m)) | ((v) & (m));

void inline bcm2835_gpio_set(uint8_t pin)
{
	BCM2835_GPIO->GPSET0 = 1 << pin;
}

void inline bcm2835_gpio_clr(uint8_t pin)
{
	BCM2835_GPIO->GPCLR0 = 1 << pin;
}

void inline bcm2835_gpio_write(uint8_t pin, uint8_t on) {
	if (on)
		bcm2835_gpio_set(pin);
	else
		bcm2835_gpio_clr(pin);
}

uint8_t inline bcm2835_gpio_lev(uint8_t pin)
{
    uint32_t value = BCM2835_GPIO->GPLEV0;
    return (value & (1 << pin)) ? HIGH : LOW;
}

void inline bcm2835_delayMicroseconds(unsigned long long micros)
{
    bcm2835_st_delay(bcm2835_st_read(), micros);
}

void inline bcm2835_gpio_pud(uint8_t pud)
{
    BCM2835_GPIO->GPPUD = pud;
}

void inline bcm2835_gpio_pudclk(uint8_t pin, uint8_t on)
{
    BCM2835_GPIO->GPPUDCLK0 =  (on ? 1 : 0) << pin;
}

void inline bcm2835_gpio_set_pud(uint8_t pin, uint8_t pud)
{
    bcm2835_gpio_pud(pud);
    bcm2835_delayMicroseconds(10);
    bcm2835_gpio_pudclk(pin, 1);
    bcm2835_delayMicroseconds(10);
    bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_pudclk(pin, 0);
}

void bcm2835_spi_begin(void)
{
    // Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
    bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); // CE1
    bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); // CE0
    bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); // MISO
    bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); // MOSI
    bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); // CLK

    BCM2835_SPI0->CS = 0; // All 0s
    BCM2835_SPI0->CS = BCM2835_SPI0_CS_CLEAR;
}

void bcm2835_spi_end(void)
{
    // Set all the SPI0 pins back to input
    bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT); // CE1
    bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT); // CE0
    bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); // MISO
    bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT); // MOSI
    bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT); // CLK
}

void inline bcm2835_spi_setBitOrder(uint8_t order)
{
    // BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one supported by SPI0
}

void inline bcm2835_spi_setClockDivider(uint16_t divider)
{
    BCM2835_SPI0->CLK = divider;
}

void inline bcm2835_spi_setDataMode(uint8_t mode)
{
    // Mask in the CPO and CPHA bits of CS
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

void inline bcm2835_spi_chipSelect(uint8_t cs)
{
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, cs, BCM2835_SPI0_CS_CS);
}

void inline bcm2835_spi_setChipSelectPolarity(uint8_t cs, uint8_t active)
{
    uint8_t shift = 21 + cs;
    // Mask in the appropriate CSPOLn bit
    BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, active << shift, 1 << shift);
}

void inline bcm2835_spi_transfernb(char* tbuf, char* rbuf, uint32_t len)
{
	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    uint32_t i;
    for (i = 0; i < len; i++)
    {
		// Maybe wait for TXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
			;

		// Write to FIFO, no barrier
		//PUT32(BCM2835_SPI0_FIFO, tbuf[i]);
		BCM2835_SPI0->FIFO = tbuf[i];

		// Wait for RXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD))
			;

		// then read the data byte
		rbuf[i] = BCM2835_SPI0->FIFO;
    }
    // Wait for DONE to be set
    while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
        	;

    // Set TA = 0, and also set the barrier
    BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}

void bcm2835_spi_transfern(char* buf, uint32_t len)
{
    bcm2835_spi_transfernb(buf, buf, len);
}

void bcm2835_spi_writenb(char* tbuf, uint32_t len)
{
    // Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    uint32_t i;
	for (i = 0; i < len; i++)
	{
		// Maybe wait for TXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
			;

		// Write to FIFO, no barrier
		BCM2835_SPI0->FIFO =  tbuf[i];
	}

    // Wait for DONE to be set
    while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
    	;

    // Set TA = 0, and also set the barrier
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}

#if 0 // moved to bcm2835_asm.S
void inline bcm2835_spi_write(uint16_t data) {
    // Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	// Maybe wait for TXD
    while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
		;

	// Write to FIFO
    BCM2835_SPI0->FIFO = data >> 8;
    BCM2835_SPI0->FIFO = data & 0x0FF;

    // Wait for DONE to be set
	while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
    	;

    // Set TA = 0, and also set the barrier
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}
#endif

void bcm2835_i2c_begin(void)
{
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_ALT0); // SDA
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_ALT0); // SCL
}

void bcm2835_i2c_end(void)
{
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_INPT); // SDA
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_INPT); // SCL
}

void bcm2835_i2c_setSlaveAddress(uint8_t addr)
{
	BCM2835_BSC1->A = addr;
}

void bcm2835_i2c_setClockDivider(uint16_t divider)
{
	BCM2835_BSC1->DIV = divider;
}

uint8_t bcm2835_i2c_write(const char * buf, uint32_t len)
{
    uint32_t remaining = len;
    uint32_t i = 0;
    uint8_t reason = BCM2835_I2C_REASON_OK;

    // Clear FIFO
    BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
    // Clear Status
    BCM2835_BSC1->S = BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE;
	// Set Data Length
    BCM2835_BSC1->DLEN = len;
    // pre populate FIFO with max buffer
	while (remaining && (i < BCM2835_BSC_FIFO_SIZE)) {
		BCM2835_BSC1 ->FIFO = buf[i];
		i++;
		remaining--;
	}

    // Enable device and start transfer
    BCM2835_BSC1->C = BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST;

	// Transfer is over when BCM2835_BSC_S_DONE
	while (!(BCM2835_BSC1 ->S & BCM2835_BSC_S_DONE)) {
		while (remaining && (BCM2835_BSC1 ->S & BCM2835_BSC_S_TXD)) {
			// Write to FIFO
			BCM2835_BSC1 ->FIFO = buf[i];
			i++;
			remaining--;
    	}
    }

	// Received a NACK
	if (BCM2835_BSC1 ->S & BCM2835_BSC_S_ERR) {
		BCM2835_BSC1 ->S = BCM2835_BSC_S_ERR;
		reason = BCM2835_I2C_REASON_ERROR_NACK;
	}

	// Received Clock Stretch Timeout
	else if (BCM2835_BSC1 ->S & BCM2835_BSC_S_CLKT) {
		reason = BCM2835_I2C_REASON_ERROR_CLKT;
	}

	// Not all data is sent
	else if (remaining) {
		reason = BCM2835_I2C_REASON_ERROR_DATA;
	}

	BCM2835_BSC1->C = BCM2835_BSC_S_DONE;

    return reason;
}

uint8_t bcm2835_i2c_read(char* buf, uint32_t len)
{
    uint32_t remaining = len;
    uint32_t i = 0;
    uint8_t reason = BCM2835_I2C_REASON_OK;

    // Clear FIFO
    BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
    // Clear Status
    BCM2835_BSC1->S = BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE;
	// Set Data Length
    BCM2835_BSC1->DLEN = len;
    // Start read
    BCM2835_BSC1->C = BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST | BCM2835_BSC_C_READ;

	// wait for transfer to complete
	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
		// we must empty the FIFO as it is populated and not use any delay
		while (BCM2835_BSC1->S & BCM2835_BSC_S_RXD) {
			// Read from FIFO, no barrier
			buf[i] = BCM2835_BSC1 ->FIFO;
			i++;
			remaining--;
		}
	}

	// transfer has finished - grab any remaining stuff in FIFO
	while (remaining && (BCM2835_BSC1 ->S & BCM2835_BSC_S_RXD)) {
		// Read from FIFO, no barrier
		buf[i] = BCM2835_BSC1 ->FIFO;
		i++;
		remaining--;
	}

	// Received a NACK
	if (BCM2835_BSC1 ->S & BCM2835_BSC_S_ERR) {
		BCM2835_BSC1 ->S = BCM2835_BSC_S_ERR;
		reason = BCM2835_I2C_REASON_ERROR_NACK;
	}

	// Received Clock Stretch Timeout
	else if (BCM2835_BSC1 ->S & BCM2835_BSC_S_CLKT) {
		reason = BCM2835_I2C_REASON_ERROR_CLKT;
	}

	// Not all data is received
	else if (remaining) {
		reason = BCM2835_I2C_REASON_ERROR_DATA;
	}

	BCM2835_BSC1->C = BCM2835_BSC_S_DONE;

    return reason;
}

void bcm2835_uart_begin(void) {
    BCM2835_UART1->ENABLE = 0x01;
    BCM2835_UART1->CNTL = 0x00;
    BCM2835_UART1->LCR = 0x03;
    BCM2835_UART1->MCR = 0x00;
    BCM2835_UART1->IER = 0x05;
    BCM2835_UART1->IIR = 0xC6;
    BCM2835_UART1->BAUD = 270;

    // Set the GPI0 pins to the Alt 5 function to enable UART1 access on them
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT5); // UART1_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT5); // UART1_RXD

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);

    // turn on the uart for send and receive
    BCM2835_UART1->CNTL = 3;
}


void inline bcm2835_uart_send(uint32_t c) {
	while ((BCM2835_UART1->LSR & 0x20) == 0)
		;
	BCM2835_UART1 ->IO = c;
}

void bcm2835_pl011_begin(void)
{
    BCM2835_PL011->CR = 0;						/* Disable everything */

    // Set the GPI0 pins to the Alt 0 function to enable PL011 access on them
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0); // PL011_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0); // PL011_RXD

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);

    /* Poll the "flags register" to wait for the UART to stop transmitting or receiving. */
	while (BCM2835_PL011 ->FR & PL011_FR_BUSY ) {
	}

    /* Flush the transmit FIFO by marking FIFOs as disabled in the "line control register". */
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;

    BCM2835_PL011->ICR = 0x7FF;					/* Clear all interrupt status */
	/*
	 * IBRD = UART_CLK / (16 * BAUD_RATE)
	 * FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
	 */
    // UART_CLK = 3000000
    // BAUD_RATE = 115200
    //BCM2835_PL011->IBRD = 1;
    //BCM2835_PL011->FBRD = 40;
    BCM2835_PL011->IBRD = PL011_BAUD_INT(115200);
    BCM2835_PL011->FBRD = PL011_BAUD_FRAC(115200);
    //BCM2835_PL011->LCRH = 0x70;					/* Set N, 8, 1, FIFO enable */
    BCM2835_PL011->LCRH = PL011_LCRH_WLEN8;		/* Set N, 8, 1, FIFO disabled */
    BCM2835_PL011->CR = 0x301;					/* Enable UART */
}

void inline bcm2835_pl011_send(uint32_t c) {
	while (1) {
		if ((BCM2835_PL011 ->FR & 0x20) == 0)
			break;
	}
	BCM2835_PL011 ->DR = c;
}

uint32_t bcm2835_mailbox_read(uint8_t channel) {

	while (1) {
		while (BCM2835_MAILBOX ->STATUS & BCM2835_MAILBOX_STATUS_RE);

		uint32_t data = BCM2835_MAILBOX ->READ;
		uint8_t read_channel = (uint8_t) (data & 0xf);

		if (read_channel == channel)
			return (data & 0xfffffff0);
	}

	return BCM2835_MAILBOX_ERROR;
}

void bcm2835_mailbox_write(uint8_t channel, uint32_t data) {
	while (BCM2835_MAILBOX->STATUS & BCM2835_MAILBOX_STATUS_WF);
	BCM2835_MAILBOX->WRITE = (data & 0xfffffff0) | (uint32_t)(channel & 0xf);
}
