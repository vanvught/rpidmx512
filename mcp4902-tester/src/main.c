#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <bcm2835.h>

#include <mcp49x2.h>

void inline bcm2835_spi_write(uint16_t data) {
    volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
    volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;

    // Clear TX and RX fifos
    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	// Maybe wait for TXD
	while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
		;

	// Write to FIFO, no barrier
	bcm2835_peri_write_nb(fifo, (data >> 8));

	// Maybe wait for TXD
	while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
		;

	// Write to FIFO, no barrier
	bcm2835_peri_write_nb(fifo, (data & 0x0FF));

    // Wait for DONE to be set
    while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
    	;

    // Set TA = 0, and also set the barrier
    bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

#define SAMPLES 128
#define DAC_MAX 0x0FF;

unsigned int chip_select = 0;

int main(int argc, char **argv) {
	int t;
	double r, d;
	uint8_t dac_a,dac_b;

	while ((argc > 1) && (argv[1][0] == '-')) {
		switch (argv[1][1]) {
		case 'c':
			sscanf (&argv[1][2], "%d", &chip_select);
			break;
		default:
			fprintf(stderr, "Warning: Unsupported argument \"%s\"!\n", argv[1]);
			exit(1);
		}
		++argv;
		--argc;
	}

	if (bcm2835_init() != 1) {
		fprintf(stderr,"Could not init bcm2835 library\n");
		exit(1);
	}

	chip_select = chip_select & 0x01;
	printf("chip select : %.1d\n", chip_select);

	bcm2835_spi_begin();

	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);

	bcm2835_spi_chipSelect(chip_select);
	bcm2835_spi_setChipSelectPolarity(chip_select, LOW);

	for (;;) {
		for (d = -0.99; d < 1; d = d + 0.01) {
			for (t = 0; t < SAMPLES; t++) {

				r = ((double) t / SAMPLES) * M_PI * 2;

				dac_a = (sin(r - (d * M_PI)) + 1) * DAC_MAX	;
				dac_b = (sin(r) + 1) * DAC_MAX;

				printf("dac_0 = %.3d, dac_1 = %.3d\n", dac_a, dac_b);

				bcm2835_spi_write(MCP4902_DATA(dac_a) | 0x3000 | MCP49X2_WRITE_DAC_A);
				bcm2835_spi_write(MCP4902_DATA(dac_b) | 0x3000 | MCP49X2_WRITE_DAC_B);

				bcm2835_delay(500);
			}
		}
	}

	return 0;
}
