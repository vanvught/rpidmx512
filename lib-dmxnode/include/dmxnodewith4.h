/**
 * @file dmxnodewith4.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODEWITH4_H_
#define DMXNODEWITH4_H_

#include <cstdint>

#include "dmxnode.h"
#include "dmxnode_outputtype.h"
#include "dmxsend.h"

#include "debug.h"

template<uint32_t nMaxPorts>
class DmxNodeWith4 {
public:
	DmxNodeWith4(DmxPixelOutputType *pA, DmxSend *pB) : m_pDmxPixelOutputType(pA), m_pDmxSend(pB) {
		DEBUG_PRINTF("nMaxPorts=%u DmxPixelOutputType=%p DmxSend=%p", nMaxPorts, reinterpret_cast<void *>(m_pDmxPixelOutputType), reinterpret_cast<void *>(m_pDmxSend));
	}

	~DmxNodeWith4() = default;

	void SetDmxPixel(DmxPixelOutputType *const pDmxPixelOutputType) {
		m_pDmxPixelOutputType = pDmxPixelOutputType;
	}

	DmxPixelOutputType *GetDmxPixel() const {
		return m_pDmxPixelOutputType;
	}

	void SetDmxSend(DmxSend *const pDmxSend) {
		m_pDmxSend = pDmxSend;
	}

	DmxSend *GetDmxSend() const {
		return m_pDmxSend;
	}

	void Start(const uint32_t nPortIndex) {
		if (nPortIndex < nMaxPorts) {
			if (m_pDmxPixelOutputType != nullptr) {
				return m_pDmxPixelOutputType->Start(nPortIndex);
			}
			return;
		}
		if (m_pDmxSend != nullptr) {
			m_pDmxSend->Start(nPortIndex & 0x3);
		}
	}

	void Stop(const uint32_t nPortIndex) {
		if (nPortIndex < nMaxPorts) {
			if (m_pDmxPixelOutputType != nullptr) {
				return m_pDmxPixelOutputType->Stop(nPortIndex);
			}
			return;
		}
		if (m_pDmxSend != nullptr) {
			m_pDmxSend->Stop(nPortIndex & 0x3);
		}
	}

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) {
		if (nPortIndex < nMaxPorts) {
			if (m_pDmxPixelOutputType != nullptr) {
				return m_pDmxPixelOutputType->SetData(nPortIndex, pData, nLength, doUpdate);
			}
			return;
		}
		if (m_pDmxSend != nullptr) {
			return m_pDmxSend->SetData(nPortIndex & 0x3, pData, nLength, doUpdate);
		}
	}

	void Sync(const uint32_t nPortIndex) {
		if (nPortIndex < nMaxPorts) {
			if (m_pDmxPixelOutputType != nullptr) {
				return m_pDmxPixelOutputType->Sync(nPortIndex);
			}
			return;
		}
		if (m_pDmxSend != nullptr) {
			return m_pDmxSend->Sync(nPortIndex & 0x3);
		}
	}

	void Sync() {
		if (m_pDmxPixelOutputType != nullptr) {
			m_pDmxPixelOutputType->Sync();
		}
		if (m_pDmxSend != nullptr) {
			m_pDmxSend->Sync();
		}
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const dmxnode::OutputStyle outputStyle) {
		if (nPortIndex < nMaxPorts) {
			if (m_pDmxPixelOutputType != nullptr) {
				return m_pDmxPixelOutputType->SetOutputStyle(nPortIndex, outputStyle);
			}
			return;
		}
		if (m_pDmxSend != nullptr) {
			return m_pDmxSend->SetOutputStyle(nPortIndex & 0x3, outputStyle);
		}
	}

	dmxnode::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const {
		if (nPortIndex < nMaxPorts) {
			if (m_pDmxPixelOutputType != nullptr) {
				return m_pDmxPixelOutputType->GetOutputStyle(nPortIndex);
			}
			return dmxnode::OutputStyle::DELTA;
		}
		if (m_pDmxSend != nullptr) {
			return m_pDmxSend->GetOutputStyle(nPortIndex & 0x3);
		}

		return dmxnode::OutputStyle::DELTA;
	}
#endif

	void Blackout(bool bBlackout) {
		if (m_pDmxPixelOutputType != nullptr) {
			m_pDmxPixelOutputType->Blackout(bBlackout);
		}
		if (m_pDmxSend != nullptr) {
			m_pDmxSend->Blackout(bBlackout);
		}
	}

	void FullOn() {
		if (m_pDmxPixelOutputType != nullptr) {
			m_pDmxPixelOutputType->FullOn();
		}
		if (m_pDmxSend != nullptr) {
			m_pDmxSend->FullOn();
		}
	}

	void Print() {
		if (m_pDmxPixelOutputType != nullptr) {
			m_pDmxPixelOutputType->Print();
		}
		if (m_pDmxSend != nullptr) {
			m_pDmxSend->Print();
		}
	}

	uint32_t GetUserData() {
		if (m_pDmxPixelOutputType != nullptr) {
			return m_pDmxPixelOutputType->GetUserData();
		}
		return 0;
	}

	uint32_t GetRefreshRate() {
		if (m_pDmxPixelOutputType != nullptr) {
			return m_pDmxPixelOutputType->GetRefreshRate();
		}
		return 0;
	}

	bool SetDmxStartAddress([[maybe_unused]] uint16_t nDmxStartAddress) {
		return false;
	}

	uint16_t GetDmxStartAddress() {
		return dmxnode::ADDRESS_INVALID;
	}

	uint16_t GetDmxFootprint() {
		return 0;
	}

	bool GetSlotInfo([[maybe_unused]] uint16_t nSlotOffset, [[maybe_unused]] dmxnode::SlotInfo &tSlotInfo) {
		return false;
	}

private:
	DmxPixelOutputType *m_pDmxPixelOutputType;
	DmxSend *m_pDmxSend;
};


#endif /* DMXNODEWITH4_H_ */
