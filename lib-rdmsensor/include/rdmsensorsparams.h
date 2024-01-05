/**
 * @file rdmsensorsparams.h
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

#ifndef RDMSENSORSPARAMS_H_
#define RDMSENSORSPARAMS_H_

#include <cstdint>

#include "rdmsensors.h"
#include "configstore.h"

namespace rdm {
namespace sensorsparams {
struct Params {
	uint32_t nDevices;
	struct {
		uint8_t nType;
		uint8_t nAddress;
		uint8_t nReserved;
	} __attribute__((packed)) Entry[rdm::sensors::devices::MAX];
	int16_t nCalibrate[rdm::sensors::MAX];
} __attribute__((packed));

static_assert(sizeof(struct Params) <= rdm::sensors::STORE, "struct Params is too large");

}  // namespace sensorsparams
}  // namespace rdm

class RDMSensorsParamsStore {
public:
	static void Update(const rdm::sensorsparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::RDMSENSORS, pParams, sizeof(struct rdm::sensorsparams::Params));
	}

	static void Copy(rdm::sensorsparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::RDMSENSORS, pParams, sizeof(struct rdm::sensorsparams::Params));
	}
};

class RDMSensorsParams {
public:
	RDMSensorsParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const rdm::sensorsparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    bool Add(RDMSensor *pRDMSensor);

private:
	rdm::sensorsparams::Params m_Params;
};

#endif /* RDMSENSORSPARAMS_H_ */
