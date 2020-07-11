/**
 * @file storerdmdevice.h
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

#ifndef STORERDMDEVICE_H_
#define STORERDMDEVICE_H_

#include <stdint.h>

#include "rdmdeviceparams.h"
#include "rdmdevicestore.h"

#include "spiflashstore.h"

class StoreRDMDevice final: public RDMDeviceParamsStore, public RDMDeviceStore {
public:
	StoreRDMDevice();

	void Update(const struct TRDMDeviceParams *pRDMDeviceParams) override {
		SpiFlashStore::Get()->Update(STORE_RDMDEVICE, pRDMDeviceParams, sizeof(struct TRDMDeviceParams));
	}

	void Copy(struct TRDMDeviceParams *pRDMDeviceParams) override {
		SpiFlashStore::Get()->Copy(STORE_RDMDEVICE, pRDMDeviceParams, sizeof(struct TRDMDeviceParams));
	}

	void SaveLabel(const char *pLabel, uint8_t nLength) override {
		SpiFlashStore::Get()->Update(STORE_RDMDEVICE, __builtin_offsetof(struct TRDMDeviceParams, aDeviceRootLabel), pLabel, nLength, RDMDeviceParamsMask::LABEL);
		SpiFlashStore::Get()->Update(STORE_RDMDEVICE, __builtin_offsetof(struct TRDMDeviceParams, nDeviceRootLabelLength), &nLength, sizeof(uint8_t), RDMDeviceParamsMask::LABEL);
	}

	static StoreRDMDevice *Get() {
		return s_pThis;
	}

private:
	static StoreRDMDevice *s_pThis;
};

#endif /* STORERDMDEVICE_H_ */
