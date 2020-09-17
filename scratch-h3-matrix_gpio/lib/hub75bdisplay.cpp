/*
 * hub75bdisplay.cpp
 *
 */
/**
 * PoC
 */

#include <stdint.h>

#include "hub75bdisplay.h"

#include "h3_gpio.h"
#include "board/h3_opi_zero.h"

#include "debug.h"

#define HUB75B_A		GPIO_EXT_13		// PA0
#define HUB75B_B		GPIO_EXT_11		// PA1
#define HUB75B_C		GPIO_EXT_22		// PA2
#define HUB75B_D		GPIO_EXT_15		// PA3

#define HUB75B_CK		GPIO_EXT_26		// PA10
#define HUB75B_LA		GPIO_EXT_7		// PA6
#define HUB75B_OE		GPIO_EXT_12		// PA7

/*
 * BAM (not PWM:)
 *
 *
 * The basic process is to
 * - blank the display,
 * - latch in the previously shifted data,
 * - update the row selects,
 * - unblank the display,
 * - shift in the next set of pixel data,
 * and then wait for an update timer to expire.
 * This is repeated four times for each row.
 * If you examine the blanking output,
 * you’ll notice that its low period doubles three times within the output period for each display row.
 * This is the result of using binary coded modulation to vary the intensity of each pixel.
 */

static uint32_t row_index;

Hub75bDisplay::Hub75bDisplay(uint32_t nColumns, uint32_t nRows): m_nColumns(nColumns), m_nRows(nRows) {
	h3_gpio_fsel(HUB75B_CK, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_LA, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_OE, GPIO_FSEL_OUTPUT);

	h3_gpio_clr(HUB75B_CK);
	h3_gpio_clr(HUB75B_LA);
	h3_gpio_set(HUB75B_OE);

	h3_gpio_fsel(HUB75B_A, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_B, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_C, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_D, GPIO_FSEL_OUTPUT);

	h3_gpio_clr(HUB75B_A);
	h3_gpio_clr(HUB75B_B);
	h3_gpio_clr(HUB75B_C);
	h3_gpio_clr(HUB75B_D);

	row_index = 0;
}

/*
 * Update LA, OE and row select
 * Just output m_nColumns clock cycles
 */
void Hub75bDisplay::Run() {
	uint32_t nGPIO = H3_PIO_PORTA->DAT;

	/* Blank the display */
	H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_OE);

	/* Latch the previous data */
	H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_LA) | (1U << HUB75B_OE);
	nGPIO |= (1U << HUB75B_OE);
	H3_PIO_PORTA->DAT = nGPIO;

	/* Update the row select */
	row_index = (row_index + 1) & 0xF;
	nGPIO &= ~(0xFU);
	nGPIO |= row_index;
	H3_PIO_PORTA->DAT = nGPIO;

	/* Enable the display */
	nGPIO &= ~(1U << HUB75B_OE);
	H3_PIO_PORTA->DAT = nGPIO;

	/* Shift in next data */
	for (uint32_t i = 0; i < m_nColumns; i++) {
		// Clock high // TODO with R1, G1, B1, R2, G2, B2 -> performance impact : read framebuffer
		H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_CK);
		// Clock low
		H3_PIO_PORTA->DAT = nGPIO;
	}
}
