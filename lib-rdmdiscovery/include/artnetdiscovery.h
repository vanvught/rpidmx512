/**
 * @file artnetdiscovery.h
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETDISCOVERY_H_
#define ARTNETDISCOVERY_H_

#include <cstdint>

#include "artnetrdm.h"

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"
#include "rdm.h"

#include "debug.h"

class ArtNetRdmController final: public RDMDeviceController, public ArtNetRdm {
public:
	ArtNetRdmController(uint32_t nPorts = artnetnode::MAX_PORTS);
	~ArtNetRdmController() override;

	void Full(uint32_t nPortIndex) override {
		DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
		assert(nPortIndex < s_nPorts);
		if (m_Discovery[nPortIndex] != nullptr) {
			m_Discovery[nPortIndex]->Full();
		}
	}

	uint32_t GetUidCount(uint32_t nPortIndex) override {
		DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
		assert(nPortIndex < s_nPorts);
		if (m_Discovery[nPortIndex] != nullptr) {
			return m_Discovery[nPortIndex]->GetUidCount();
		}
		return 0;
	}

	void Copy(uint32_t nPortIndex, uint8_t *pTod) override {
		DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
		assert(nPortIndex < s_nPorts);
		if (m_Discovery[nPortIndex] != nullptr) {
			m_Discovery[nPortIndex]->Copy(pTod);
		}
	}

	const uint8_t *Handler(uint32_t nPortIndex, const uint8_t *pRdmData) override;

	void Print() {
		RDMDeviceController::Print();
	}

	void DumpTod(uint32_t nPortIndex) {
		DEBUG_PRINTF("nPortIndex=%d", nPortIndex);
		assert(nPortIndex < s_nPorts);
		if (m_Discovery[nPortIndex] != nullptr) {
			m_Discovery[nPortIndex]->Dump();
		}
	}

private:
	RDMDiscovery *m_Discovery[artnetnode::MAX_PORTS];
	static TRdmMessage s_rdmMessage;
	static uint32_t s_nPorts;
};

#endif /* ARTNETDISCOVERY_H_ */
