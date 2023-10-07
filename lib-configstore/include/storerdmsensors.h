/**
 * @file storerdmsensors.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "rdmsensorsparams.h"
#include "rdmsensorstore.h"
#include "rdmsensors.h"

#include "configstore.h"

#include "debug.h"	//FIXME Remove

class StoreRDMSensors final: public RDMSensorsParamsStore, public RDMSensorStore {
public:
	StoreRDMSensors();

	void Update(const rdm::sensorsparams::Params *pParams) override {
		ConfigStore::Get()->Update(configstore::Store::RDMSENSORS, pParams, sizeof(struct rdm::sensorsparams::Params));
	}

	void Copy(rdm::sensorsparams::Params *pParams) override {
		ConfigStore::Get()->Copy(configstore::Store::RDMSENSORS, pParams, sizeof(struct rdm::sensorsparams::Params));
	}

	 void SaveCalibration(uint32_t nSensor, int32_t nCalibration) override {
		 assert(nSensor < rdm::sensors::MAX);
		 DEBUG_PRINTF("nSensor=%u, nCalibration=%d", nSensor, nCalibration);
		 auto c = static_cast<int16_t>(nCalibration);
		 ConfigStore::Get()->Update(configstore::Store::RDMSENSORS, (nSensor * sizeof(int16_t)) + __builtin_offsetof(struct rdm::sensorsparams::Params, nCalibrate), &c, sizeof(int16_t));
	 }

	static StoreRDMSensors *Get() {
		return s_pThis;
	}

private:
	static StoreRDMSensors *s_pThis;
};

#endif /* STORERDMSENSORS_H_ */
