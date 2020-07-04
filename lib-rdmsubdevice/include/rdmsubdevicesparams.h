/**
 * @file rdmsubdevicesparams.h
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

#ifndef RDMSUBDEVICESPARAMS_H_
#define RDMSUBDEVICESPARAMS_H_

#include <stdint.h>

#include "rdmsubdevices.h"

struct TRDMSubDevicesParams {
	uint32_t nCount;
	struct {
		uint8_t nType;
		uint8_t nChipSelect;
		uint8_t nAddress;
		uint16_t nDmxStartAddress;
		uint32_t nSpeedHz;
	} __attribute__((packed)) Entry[rdm::subdevices::max];
} __attribute__((packed));

static_assert(sizeof(struct TRDMSubDevicesParams) <= rdm::subdevices::store, "too big");

class RDMSubDevicesParamsStore {
public:
	virtual ~RDMSubDevicesParamsStore() {
	}

	virtual void Update(const struct TRDMSubDevicesParams *pTRDMSubDevicesParams)=0;
	virtual void Copy(struct TRDMSubDevicesParams *pTRDMSubDevicesParams)=0;
};

class RDMSubDevicesParams {
public:
	RDMSubDevicesParams(RDMSubDevicesParamsStore *pRDMSubDevicesParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TRDMSubDevicesParams *pRDMSubDevicesParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Dump();

	void Set();

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool Add(RDMSubDevice *pRDMSubDevice);

private:
    RDMSubDevicesParamsStore *m_pRDMSubDevicesParamsStore;
	TRDMSubDevicesParams m_tRDMSubDevicesParams;
};

#endif /* RDMSUBDEVICESPARAMS_H_ */
