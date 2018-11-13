/**
 * @file w5500.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define _WIZCHIP_ 5500
#include "wizchip_conf.h"

#if defined(BARE_METAL)
 #include "bcm2835_spi.h"
#else
 #include "bcm2835.h"
 #define BCM2835_SPI_CLOCK_MIN	4000			///< 4kHz
 #define BCM2835_SPI_CLOCK_MAX	125000000		///< 125Mhz
#endif

#include "w5x00.h"

extern int bcm2835_prereq(void);
extern void w5x00_netinfo_set(wiz_NetInfo *netinfo, const uint8_t *pMacAddress, const struct ip_info *pIpInfo);
extern void w5x00_netinfo_dump(const wiz_NetInfo *netinfo);

static uint8_t w5500_spi_cs;
static uint16_t w5500_spi_clk_div;

static void w5500_cs_select(void) {
	__sync_synchronize();
	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
	bcm2835_spi_setClockDivider(w5500_spi_clk_div);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	__sync_synchronize();
	bcm2835_gpio_clr(w5500_spi_cs);
	__sync_synchronize();
}

static void w5500_cs_deselect(void) {
	__sync_synchronize();
	bcm2835_gpio_set(w5500_spi_cs);
	__sync_synchronize();
}

static void w5500_read_burst(uint8_t *pBuf, uint16_t len) {
	bcm2835_spi_transfern((char*) pBuf, len);
	return;
}

static void w5500_write_burst(uint8_t *pBuf, uint16_t len) {
	bcm2835_spi_writenb((const char*) pBuf, len);
}

static void w5500_write(uint8_t b) {
	bcm2835_spi_writenb((const char*) &b, 1);
}

static uint8_t w5500_read(void) {
	uint8_t rbuf;
	bcm2835_spi_transfern((char*) &rbuf, 1);
	return rbuf;
}

W5X00_Init w5500_init(uint8_t nSpiChipSelect, uint32_t nSpiSpeedHz, const uint8_t *pMacAddress, struct ip_info *pIpInfo) {
	int rc = bcm2835_prereq();

	if (rc < 0) {
		return rc;
	}

	assert(pMacAddress != 0);
	assert(pIpInfo != 0);

	assert(nSpiChipSelect < 32);
	w5500_spi_cs = nSpiChipSelect;

	if ((nSpiSpeedHz < BCM2835_SPI_CLOCK_MIN) || (nSpiSpeedHz > BCM2835_SPI_CLOCK_MAX)) {
		w5500_spi_clk_div = (uint16_t) BCM2835_SPI_CLOCK_DIVIDER_64;
	} else {
		w5500_spi_clk_div = (uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / nSpiSpeedHz);
	}

	bcm2835_spi_begin();

	bcm2835_gpio_fsel(w5500_spi_cs, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(w5500_spi_cs);

	reg_wizchip_cs_cbfunc(w5500_cs_select, w5500_cs_deselect);
	reg_wizchip_spiburst_cbfunc(w5500_read_burst, w5500_write_burst);
	reg_wizchip_spi_cbfunc(w5500_read, w5500_write);

	wizphy_reset();

	uint8_t bufSize[2][8] = { {8,2,2,2,2,0,0,0},{8,2,2,2,2,0,0,0}};

	if (wizchip_init(bufSize[0], bufSize[1]) != 0) {
		return W5X00_INIT_WIZCHIP_INIT_FAILED;
	}

	do {
		rc = wizphy_getphylink();
		if (rc == -1) {
			return W5X00_INIT_WIZPHY_GET_FAILED;
		}
	} while (rc == PHY_LINK_OFF);

	wiz_NetInfo netInfo;

	w5x00_netinfo_set(&netInfo, pMacAddress, pIpInfo);

	wizchip_setnetinfo(&netInfo);

#ifndef NDEBUG
	wizchip_getnetinfo(&netInfo);
	w5x00_netinfo_dump(&netInfo);
#endif

	return W5X00_INIT_OK;
}

W5X00_Init w5x00_init(uint8_t nSpiChipSelect, uint32_t nSpiSpeedHz, const uint8_t *pMacAddress, struct ip_info *pIpInfo) {
	return w5500_init(nSpiChipSelect, nSpiSpeedHz, pMacAddress, pIpInfo);
}

const char *w5x00_get_chip_name(void) {
	return _WIZCHIP_ID_;
}
