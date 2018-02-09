/**
 * @file bcm2835_aux_spi.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BCM2835_AUX_SPI_H_
#define BCM2835_AUX_SPI_H_

#include <stdint.h>

#define BCM2835_AUX_SPI_CLOCK_MIN	30500			///< 30,5kHz
#define BCM2835_AUX_SPI_CLOCK_MAX	125000000		///< 125Mhz

#ifdef __cplusplus
extern "C" {
#endif

extern void bcm2835_aux_spi_begin(void);

extern void bcm2835_aux_spi_setClockDivider(uint16_t);
extern const uint16_t bcm2835_aux_spi_CalcClockDivider(uint32_t);

extern void bcm2835_aux_spi_write(uint16_t);
extern void bcm2835_aux_spi_writenb(const char *, uint32_t);

extern void bcm2835_aux_spi_transfernb(const char *, /*@null@*/char *, uint32_t);
extern void bcm2835_aux_spi_transfern(char *, uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* BCM2835_AUX_SPI_H_ */
