/**
 * @file rdmsubdevicesparams.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "rdmsubdevices.h"
#include "configstore.h"

namespace rdm::subdevicesparams {
struct Params {
	uint32_t nCount;
	struct {
		uint8_t nType;
		uint8_t nChipSelect;
		uint8_t nAddress;
		uint16_t nDmxStartAddress;
		uint32_t nSpeedHz;
	} __attribute__((packed)) Entry[rdm::subdevices::MAX];
} __attribute__((packed));

static_assert(sizeof(struct Params) <= configstore::STORE_SIZE[static_cast<uint32_t>(configstore::Store::RDMSUBDEVICES)]);

} // namespace rdm::subdevicesparams

class RDMSubDevicesParams {
public:
	RDMSubDevicesParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const rdm::subdevicesparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void CallbackFunction(const char *pLine);
    bool Add(RDMSubDevice *pRDMSubDevice);

private:
    rdm::subdevicesparams::Params m_Params;
};

#endif /* RDMSUBDEVICESPARAMS_H_ */
