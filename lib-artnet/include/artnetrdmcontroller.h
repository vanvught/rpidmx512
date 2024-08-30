/**
 * @file artnetrdmcontroller.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETRDMCONTROLLER_H_
#define ARTNETRDMCONTROLLER_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"
#include "rdm.h"

#include "artnetnode_ports.h"

#include "debug.h"

class ArtNetRdmController final: public RDMDeviceController, RDMDiscovery {
public:
	ArtNetRdmController(): RDMDiscovery(RDMDeviceController::GetUID()) {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	// Discovery

	void Full(const uint32_t nPortIndex) {
		DEBUG_ENTRY
		assert(nPortIndex < artnetnode::MAX_PORTS);
		RDMDiscovery::Full(nPortIndex, &m_pRDMTod[nPortIndex]);
		DEBUG_EXIT
	}

	void Incremental(const uint32_t nPortIndex) {
		DEBUG_ENTRY
		assert(nPortIndex < artnetnode::MAX_PORTS);
		RDMDiscovery::Incremental(nPortIndex, &m_pRDMTod[nPortIndex]);
		DEBUG_EXIT
	}

	void Stop(const uint32_t nPortIndex) {
		DEBUG_ENTRY
		assert(nPortIndex < artnetnode::MAX_PORTS);
		bool bIsIncremental;
		uint32_t _nPortIndex;
		if (IsRunning(_nPortIndex, bIsIncremental)) {
			if (_nPortIndex == nPortIndex) {
				RDMDiscovery::Stop();
			}
		}
		DEBUG_EXIT
	}

	uint32_t GetUidCount(const uint32_t nPortIndex) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_pRDMTod[nPortIndex].GetUidCount();
	}

	void TodCopy(const uint32_t nPortIndex, uint8_t *pTod) {
		DEBUG_ENTRY
		assert(nPortIndex < artnetnode::MAX_PORTS);
		m_pRDMTod[nPortIndex].Copy(pTod);
		DEBUG_EXIT
	}

	void Run() {
		RDMDiscovery::Run();
	}

	bool IsRunning(uint32_t& nPortIndex, bool& bIsIncremental) {
		return RDMDiscovery::IsRunning(nPortIndex, bIsIncremental);
	}

	bool IsFinished(uint32_t& nPortIndex, bool& bIsIncremental) {
		return RDMDiscovery::IsFinished(nPortIndex, bIsIncremental);
	}

	uint32_t CopyWorkingQueue(char *pOutBuffer, const uint32_t nOutBufferSize) {
		return RDMDiscovery::CopyWorkingQueue(pOutBuffer, nOutBufferSize);
	}

	uint32_t CopyTod(const uint32_t nPortIndex, char *pOutBuffer, const uint32_t nOutBufferSize) {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		const auto nSize = static_cast<int32_t>(nOutBufferSize);
		int32_t nLength = 0;

		for (uint32_t nCount = 0; nCount < m_pRDMTod[nPortIndex].GetUidCount(); nCount++) {
			uint8_t uid[RDM_UID_SIZE];

			m_pRDMTod[nPortIndex].CopyUidEntry(nCount, uid);

			nLength += snprintf(&pOutBuffer[nLength], static_cast<size_t>(nSize - nLength),
					"\"%.2x%.2x:%.2x%.2x%.2x%.2x\",",
					uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);
		}

		if (nLength == 0) {
			return 0;
		}

		pOutBuffer[nLength - 1] = '\0';

		return static_cast<uint32_t>(nLength - 1);

	}

	// Gateway

	bool RdmReceive(const uint32_t nPortIndex, const uint8_t *pRdmData);

	void TodReset(const uint32_t nPortIndex) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		m_pRDMTod[nPortIndex].Reset();
	}

	bool TodAddUid(const uint32_t nPortIndex, const uint8_t *pUid) {
		return m_pRDMTod[nPortIndex].AddUid(pUid);
	}

	// Generic

	bool CopyTodEntry(const uint32_t nPortIndex, uint32_t nIndex, uint8_t uid[RDM_UID_SIZE]) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_pRDMTod[nPortIndex].CopyUidEntry(nIndex, uid);
	}

	void TodDump(const uint32_t nPortIndex) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		m_pRDMTod[nPortIndex].Dump();
	}

	RDMTod *GetTod(const uint32_t nPortIndex) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return &m_pRDMTod[nPortIndex];
	}

private:
	static RDMTod m_pRDMTod[artnetnode::MAX_PORTS];
};

#endif /* ARTNETRDMCONTROLLER_H_ */
