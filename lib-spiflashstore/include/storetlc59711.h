/**
 * @file storetlc59711.h
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

#ifndef STORETLC59711_H_
#define STORETLC59711_H_

#include <stdint.h>

#include "tlc59711dmxparams.h"
#include "tlc59711dmxstore.h"

#include "spiflashstore.h"

class StoreTLC59711 final: public TLC59711DmxParamsStore, public TLC59711DmxStore {
public:
	StoreTLC59711();

	void Update(const struct TTLC59711DmxParams *pTLC59711DmxParams) override {
		SpiFlashStore::Get()->Update(STORE_TLC5711DMX, pTLC59711DmxParams, sizeof(struct TTLC59711DmxParams));
	}

	void Copy(struct TTLC59711DmxParams *pTLC59711DmxParams) override {
		SpiFlashStore::Get()->Copy(STORE_TLC5711DMX, pTLC59711DmxParams, sizeof(struct TTLC59711DmxParams));
	}

	void SaveDmxStartAddress(uint16_t nDmxStartAddress) override {
		SpiFlashStore::Get()->Update(STORE_TLC5711DMX, __builtin_offsetof(struct TTLC59711DmxParams, nDmxStartAddress), &nDmxStartAddress, sizeof(uint32_t), TLC59711DmxParamsMask::START_ADDRESS);
	}

	static StoreTLC59711 *Get() {
		return s_pThis;
	}

private:
	static StoreTLC59711 *s_pThis;
};

#endif /* STORETLC59711_H_ */
