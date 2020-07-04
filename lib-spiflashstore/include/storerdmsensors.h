/**
 * @file storerdmsensors.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STORERDMSENSORS_H_
#define STORERDMSENSORS_H_

#include "rdmsensorsparams.h"

#include "spiflashstore.h"

class StoreRDMSensors final: public RDMSensorsParamsStore {
public:
	StoreRDMSensors();

	void Update(const struct TRDMSensorsParams *pRDMSensorsParams) override {
		SpiFlashStore::Get()->Update(STORE_RDMSENSORS, pRDMSensorsParams, sizeof(struct TRDMSensorsParams));
	}

	void Copy(struct TRDMSensorsParams *pRDMSensorsParams) override {
		SpiFlashStore::Get()->Copy(STORE_RDMSENSORS, pRDMSensorsParams, sizeof(struct TRDMSensorsParams));
	}

	static StoreRDMSensors *Get() {
		return s_pThis;
	}

private:
	static StoreRDMSensors *s_pThis;
};

#endif /* STORERDMSENSORS_H_ */
