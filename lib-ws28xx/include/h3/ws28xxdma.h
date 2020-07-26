/**
 * @file ws28xxdma.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XXDMA_H_
#define WS28XXDMA_H_

#include <stdint.h>

#include "ws28xx.h"

#include "h3_spi.h"

class WS28xxDMA: public WS28xx {
public:
	WS28xxDMA(TWS28XXType Type, uint16_t nLedCount, TRGBMapping tRGBMapping = RGB_MAPPING_UNDEFINED, uint8_t nT0H = 0, uint8_t nT1H = 0, uint32_t nClockSpeed = spi::speed::ws2801::default_hz);
	~WS28xxDMA();

	bool Initialize ();

	void Update();
	void Blackout();

	bool IsUpdating () { // returns TRUE while DMA operation is active
		return h3_spi_dma_tx_is_active();
	}
};

#endif /* WS28XXDMA_H_ */
