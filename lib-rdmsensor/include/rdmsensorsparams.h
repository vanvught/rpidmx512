/**
 * @file rdmsensorsparams.h
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

#ifndef RDMSENSORSPARAMS_H_
#define RDMSENSORSPARAMS_H_

#include <stdint.h>

#include "rdmsensors.h"

struct TRDMSensorsParams {
	uint32_t nCount;
	struct {
		uint8_t nType;
		uint8_t nAddress;
		uint8_t nReserved;
	} __attribute__((packed)) Entry[rdm::sensors::max];
} __attribute__((packed));

static_assert(sizeof(struct TRDMSensorsParams) <= rdm::sensors::store, "too big");

class RDMSensorsParamsStore {
public:
	virtual ~RDMSensorsParamsStore() {
	}

	virtual void Update(const struct TRDMSensorsParams *pRDMSensorsParams)=0;
	virtual void Copy(struct TRDMSensorsParams *pRDMSensorsParams)=0;
};

class RDMSensorsParams {
public:
	RDMSensorsParams(RDMSensorsParamsStore *pRDMSensorsParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TRDMSensorsParams *pRDMSensorsParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Dump();

	void Set();

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool Add(RDMSensor *pRDMSensor);

private:
	RDMSensorsParamsStore *m_pRDMSensorsParamsStore;
	TRDMSensorsParams m_tRDMSensorsParams;
};

#endif /* RDMSENSORSPARAMS_H_ */
