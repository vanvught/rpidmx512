/**
 * @file storews28xxdmx.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STOREWS28XXDMX_H_
#define STOREWS28XXDMX_H_

#include <stdint.h>

#include "ws28xxdmxparams.h"
#include "ws28xxdmxstore.h"

#include "spiflashstore.h"

class StoreWS28xxDmx final: public WS28xxDmxParamsStore, public WS28xxDmxStore {
public:
	StoreWS28xxDmx();

	void Update(const struct TWS28xxDmxParams *pWS28xxDmxParams) override {
		SpiFlashStore::Get()->Update(STORE_WS28XXDMX, pWS28xxDmxParams, sizeof(struct TWS28xxDmxParams));
	}

	void Copy(struct TWS28xxDmxParams *pWS28xxDmxParams) override {
		SpiFlashStore::Get()->Copy(STORE_WS28XXDMX, pWS28xxDmxParams, sizeof(struct TWS28xxDmxParams));
	}

	void SaveDmxStartAddress(uint16_t nDmxStartAddress) override {
		SpiFlashStore::Get()->Update(STORE_WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nDmxStartAddress), &nDmxStartAddress, sizeof(uint32_t), WS28xxDmxParamsMask::DMX_START_ADDRESS);
	}

	static StoreWS28xxDmx *Get() {
		return s_pThis;
	}

private:
	static StoreWS28xxDmx *s_pThis;
};

#endif /* STOREWS28XXDMX_H_ */
