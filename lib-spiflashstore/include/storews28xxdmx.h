/**
 * @file storews28xxdmx.h
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "ws28xxdmxparams.h"
#include "ws28xxdmxstore.h"

#include "spiflashstore.h"

class StoreWS28xxDmx final: public WS28xxDmxParamsStore, public WS28xxDmxStore {
public:
	StoreWS28xxDmx();

	void Update(const struct TWS28xxDmxParams *pWS28xxDmxParams) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, pWS28xxDmxParams, sizeof(struct TWS28xxDmxParams));
	}

	void Copy(struct TWS28xxDmxParams *pWS28xxDmxParams) override {
		SpiFlashStore::Get()->Copy(spiflashstore::Store::WS28XXDMX, pWS28xxDmxParams, sizeof(struct TWS28xxDmxParams));
	}

	void SaveType(uint8_t nType) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nType), &nType, sizeof(uint8_t), WS28xxDmxParamsMask::TYPE);
	}

	void SaveCount(uint16_t nCount) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nCount), &nCount, sizeof(uint16_t), WS28xxDmxParamsMask::COUNT);
	}

	void SaveGroupingCount(uint16_t nGroupingCount) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nGroupingCount), &nGroupingCount, sizeof(uint16_t), WS28xxDmxParamsMask::GROUPING_COUNT);
	}

	void SaveMap(uint8_t nMap) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nMap), &nMap, sizeof(uint8_t), WS28xxDmxParamsMask::MAP);
	}

	void SaveTestPattern(uint8_t nTestPattern) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nTestPattern), &nTestPattern, sizeof(uint8_t), WS28xxDmxParamsMask::TEST_PATTERN);
	}

	void SaveDmxStartAddress(uint16_t nDmxStartAddress) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::WS28XXDMX, __builtin_offsetof(struct TWS28xxDmxParams, nDmxStartAddress), &nDmxStartAddress, sizeof(uint16_t), WS28xxDmxParamsMask::DMX_START_ADDRESS);
	}

	static StoreWS28xxDmx *Get() {
		return s_pThis;
	}

private:
	static StoreWS28xxDmx *s_pThis;
};

#endif /* STOREWS28XXDMX_H_ */
