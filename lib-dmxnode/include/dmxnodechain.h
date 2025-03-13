/**
 * @file dmxnodechain.h
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

#ifndef DMXNODECHAIN_H_
#define DMXNODECHAIN_H_

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "dmxnode.h"
#include "sparkfundmx.h"
#include "tlc59711dmx.h"

#include "debug.h"

class DmxNodeChain {
public:
	DmxNodeChain() = default;
	~DmxNodeChain() = default;

	void SetSparkfunDmx(SparkFunDmx *pSparkFunDmx) {
		DEBUG_ENTRY
		assert(pSparkFunDmx != nullptr);
		m_pSparkFunDmx = pSparkFunDmx;

		Set<SparkFunDmx>(pSparkFunDmx);
		DEBUG_EXIT
	}

	void SetTLC59711Dmx(TLC59711Dmx *pTLC59711Dmx) {
		DEBUG_ENTRY
		m_pTLC59711Dmx = pTLC59711Dmx;

		if (m_pTLC59711Dmx == nullptr) {
			DEBUG_EXIT
			return;
		}

		Set<TLC59711Dmx>(pTLC59711Dmx);
		DEBUG_EXIT
	}

	void Start(const uint32_t nPortIndex) {
		assert(m_pSparkFunDmx != nullptr);
		m_pSparkFunDmx->Start(nPortIndex);

		if (m_pTLC59711Dmx != nullptr) {
			m_pTLC59711Dmx->Start(nPortIndex);
		}
	}

	void Stop(const uint32_t nPortIndex) {
		assert(m_pSparkFunDmx != nullptr);
		m_pSparkFunDmx->Stop(nPortIndex);

		if (m_pTLC59711Dmx != nullptr) {
			m_pTLC59711Dmx->Stop(nPortIndex);
		}
	}

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) {
		assert(pData != nullptr);
		assert(m_pSparkFunDmx != nullptr);
		m_pSparkFunDmx->SetData(nPortIndex, pData, nLength, doUpdate);

		if (m_pTLC59711Dmx != nullptr) {
			m_pTLC59711Dmx->SetData(nPortIndex, pData, nLength, doUpdate);
		}
	}

	void Sync(const uint32_t nPortIndex) {
		assert(m_pSparkFunDmx != nullptr);
		m_pSparkFunDmx->Sync(nPortIndex);

		if (m_pTLC59711Dmx != nullptr) {
			m_pTLC59711Dmx->Sync(nPortIndex);
		}
	}

	void Sync() {
		assert(m_pSparkFunDmx != nullptr);
		m_pSparkFunDmx->Sync();

		if (m_pTLC59711Dmx != nullptr) {
			m_pTLC59711Dmx->Sync();
		}
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle([[maybe_unused]] const uint32_t nPortIndex, [[maybe_unused]] const dmxnode::OutputStyle outputStyle) {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	dmxnode::OutputStyle GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const {
		return dmxnode::OutputStyle::DELTA;
	}
#endif

	 uint16_t GetDmxFootprint()  {
		return m_nDmxFootprint;
	}

	bool SetDmxStartAddress(uint16_t nDmxStartAddress) {
		DEBUG_ENTRY

		if (nDmxStartAddress == m_nDmxStartAddress) {
			DEBUG_EXIT
			return true;
		}

		const auto nCurrentDmxStartAddress = m_pSparkFunDmx->GetDmxStartAddress();
		const auto nNewDmxStartAddress =  static_cast<uint16_t>((nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress);
		m_pSparkFunDmx->SetDmxStartAddress(nNewDmxStartAddress);

		if (m_pTLC59711Dmx != nullptr) {
			const auto nCurrentDmxStartAddress = m_pTLC59711Dmx->GetDmxStartAddress();
			const auto nNewDmxStartAddress =  static_cast<uint16_t>((nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress);
			m_pTLC59711Dmx->SetDmxStartAddress(nNewDmxStartAddress);
		}

		m_nDmxStartAddress = nDmxStartAddress;

		DEBUG_EXIT
		return true;
	}

	 uint16_t GetDmxStartAddress() {
		return m_nDmxStartAddress;
	}

	bool GetSlotInfo(const uint16_t nSlotOffset, dmxnode::SlotInfo &slotOffset) {
		DEBUG_ENTRY

		if (nSlotOffset > m_nDmxFootprint) {
			DEBUG_EXIT
			return false;
		}

		auto b = GetSlotInfo<SparkFunDmx>(m_pSparkFunDmx, nSlotOffset, slotOffset);

		if (b) {
			DEBUG_EXIT
			return true;
		}

		b = (m_pTLC59711Dmx != nullptr) && GetSlotInfo<TLC59711Dmx>(m_pTLC59711Dmx, nSlotOffset, slotOffset);

		DEBUG_EXIT
		return b;
	}

	uint32_t GetUserData() { return 0; }		///< Art-Net ArtPollReply
	uint32_t GetRefreshRate() { return 0; }		///< Art-Net ArtPollReply

	void Blackout([[maybe_unused]] bool bBlackout) {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void FullOn() {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void Print() {
		m_pSparkFunDmx->Print();
		if (m_pTLC59711Dmx != nullptr) {
			m_pTLC59711Dmx->Print();
		}
	}

private:
	template<class T>
	void Set(T *pT) {
		assert(pT != nullptr);

		[[maybe_unused]] const bool IsValidDmxStartAddress = (pT->GetDmxStartAddress() > 0) && (static_cast<uint32_t>(pT->GetDmxFootprint() - pT->GetDmxStartAddress()) < dmxnode::UNIVERSE_SIZE);
		assert(IsValidDmxStartAddress);

		if (m_nDmxStartAddress == dmxnode::ADDRESS_INVALID) {
			m_nDmxStartAddress = pT->GetDmxStartAddress();
			m_nDmxFootprint = pT->GetDmxFootprint();

			DEBUG_PRINTF("m_nDmxStartAddress=%d, m_nDmxFootprint=%d", m_nDmxStartAddress, m_nDmxFootprint);
			DEBUG_EXIT
			return;
		}

		DEBUG_PRINTF("pT->GetDmxStartAddress()=%d, pT->GetDmxFootprint()=%d\n", pT->GetDmxStartAddress(), pT->GetDmxFootprint());

		const auto nDmxChannelLastCurrent = static_cast<uint16_t>(m_nDmxStartAddress + m_nDmxFootprint);
		m_nDmxStartAddress = std::min(m_nDmxStartAddress, pT->GetDmxStartAddress());

		const auto nDmxChannelLast = static_cast<uint16_t>(pT->GetDmxStartAddress() + pT->GetDmxFootprint());
		m_nDmxFootprint = static_cast<uint16_t>(std::max(nDmxChannelLastCurrent, nDmxChannelLast) - m_nDmxStartAddress);

		DEBUG_PRINTF("m_nDmxStartAddress=%d, m_nDmxFootprint=%d\n", m_nDmxStartAddress, m_nDmxFootprint);
	}

	template<class T>
	bool GetSlotInfo(T *pT, const uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) {
		assert(pT != nullptr);

		const auto nDmxAddress = m_nDmxStartAddress + nSlotOffset;
		const auto nOffset = static_cast<int32_t>(nDmxAddress - pT->GetDmxStartAddress());

#ifndef NDEBUG
		printf("\tnSlotOffset=%d, m_nDmxStartAddress=%d, pT->GetDmxStartAddress()=%d, pT->GetDmxFootprint()=%d\n",
				static_cast<int>(nSlotOffset),
				static_cast<int>(m_nDmxStartAddress),
				static_cast<int>(pT->GetDmxStartAddress()),
				static_cast<int>(pT->GetDmxFootprint()));

		printf("\tnOffset=%d\n", nOffset);
#endif

		if ((pT->GetDmxStartAddress() + pT->GetDmxFootprint() <= nDmxAddress) || (nOffset < 0)){
			DEBUG_EXIT
			return false;
		}

		const auto b = pT->GetSlotInfo(static_cast<uint16_t>(nOffset), slotInfo);
		DEBUG_EXIT
		return b;
	}

private:
	SparkFunDmx *m_pSparkFunDmx { nullptr };
	TLC59711Dmx *m_pTLC59711Dmx { nullptr };
	uint16_t m_nDmxStartAddress { dmxnode::ADDRESS_INVALID };
	uint16_t m_nDmxFootprint { 0 };
};

#endif /* DMXNODECHAIN_H_ */
