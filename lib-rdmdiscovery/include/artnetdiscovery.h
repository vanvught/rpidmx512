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
#include <cstdio>
#include <cassert>

#include "artnetrdm.h"

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"
#include "rdm.h"

class ArtNetRdmController final: public RDMDeviceController, public ArtNetRdm, RDMDiscovery {
public:
	ArtNetRdmController(uint32_t nPorts = artnetnode::MAX_PORTS);

	~ArtNetRdmController() override {
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if (m_pRDMTod[nPortIndex] != nullptr) {
				delete m_pRDMTod[nPortIndex];
				m_pRDMTod[nPortIndex] = nullptr;
			}
		}
	}

	void Full(uint32_t nPortIndex) override {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		RDMDiscovery::Full(nPortIndex, m_pRDMTod[nPortIndex]);
	}

	uint32_t GetUidCount(uint32_t nPortIndex) override {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if (m_pRDMTod[nPortIndex] != nullptr) {
			return m_pRDMTod[nPortIndex]->GetUidCount();
		}
		return 0;
	}

	void Copy(uint32_t nPortIndex, uint8_t *pTod) override {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if (m_pRDMTod[nPortIndex] != nullptr) {
			m_pRDMTod[nPortIndex]->Copy(pTod);
		}
	}

	const uint8_t *Handler(uint32_t nPortIndex, const uint8_t *pRdmData) override;

	bool CopyTodEntry(uint32_t nPortIndex, uint32_t nIndex, uint8_t uid[RDM_UID_SIZE]) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if (m_pRDMTod[nPortIndex] == nullptr) {
			memcpy(uid, UID_ALL, RDM_UID_SIZE);
			return false;
		}

		return m_pRDMTod[nPortIndex]->CopyUidEntry(nIndex, uid);
	}

	void Print() {
		RDMDeviceController::Print();
	}

	void DumpTod(uint32_t nPortIndex) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if (m_pRDMTod[nPortIndex] != nullptr) {
			m_pRDMTod[nPortIndex]->Dump();
		}
	}

private:
	static RDMTod *m_pRDMTod[artnetnode::MAX_PORTS];
	static TRdmMessage s_rdmMessage;
	static uint32_t s_nPorts;
};

#endif /* ARTNETDISCOVERY_H_ */
