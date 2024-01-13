/**
 * @file rdmdeviceparams.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMDEVICEPARAMS_H_
#define RDMDEVICEPARAMS_H_

#include <cstdint>

#include "rdmconst.h"
#include "configstore.h"

namespace rdm {
namespace deviceparams {
struct Params {
    uint32_t nSetList;
    char aDeviceRootLabel[RDM_DEVICE_LABEL_MAX_LENGTH];
	uint8_t nDeviceRootLabelLength;
	uint16_t nProductCategory;
	uint16_t nProductDetail;
} __attribute__((packed));

struct Mask {
	static constexpr auto LABEL = (1U << 0);
	static constexpr auto PRODUCT_CATEGORY = (1U << 1);
	static constexpr auto PRODUCT_DETAIL = (1U << 2);
};
}  // namespace rdmdevice
}  // namespace rdm

class RDMDevice;

class RDMDeviceParamsStore {
public:
	static void Update(const struct rdm::deviceparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::RDMDEVICE, pParams, sizeof(struct rdm::deviceparams::Params));
	}

	static void Copy(struct rdm::deviceparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::RDMDEVICE, pParams, sizeof(struct rdm::deviceparams::Params));
	}
};

class RDMDeviceParams {
public:
	RDMDeviceParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct rdm::deviceparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(RDMDevice *pRDMDevice);

    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    rdm::deviceparams::Params m_Params;
};

#endif /* RDMDEVICEPARAMS_H_ */
