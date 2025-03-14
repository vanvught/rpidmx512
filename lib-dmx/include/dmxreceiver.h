/**
 * @file dmxreceiver.h
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXRECEIVER_H
#define DMXRECEIVER_H

#include <cstdint>
#include <cstdio>

#include "dmx.h"
#include "dmxnode_outputtype.h"

#include "hal_statusled.h"

class DMXReceiver: Dmx {
public:
	DMXReceiver(DmxNodeOutputType *pDmxNodeOutputType) {
		m_pDmxNodeOutputType = pDmxNodeOutputType;
	}

	~DMXReceiver() {
		DMXReceiver::Stop();
		m_IsActive = false;
	}

	void Start() {
		Dmx::SetPortDirection(0, dmx::PortDirection::INP, true);
	}

	void Stop() {
		Dmx::SetPortDirection(0, dmx::PortDirection::INP, false);
		m_pDmxNodeOutputType->Stop(0);
	}

	void SetDmxNodeOutputType(DmxNodeOutputType *pDmxNodeOutputType) {
		if (pDmxNodeOutputType != m_pDmxNodeOutputType) {
			m_pDmxNodeOutputType->Stop(0);
			m_pDmxNodeOutputType = pDmxNodeOutputType;
			m_IsActive = false;
		}

	}

	const uint8_t *Run(int16_t &nLength) {
		if (__builtin_expect((m_bDisableOutput), 0)) {
			nLength = 0;
			return nullptr;
		}

		if (Dmx::GetDmxUpdatesPerSecond(0) == 0) {
			if (m_IsActive) {
				m_pDmxNodeOutputType->Stop(0);
				m_IsActive = false;
				hal::statusled_set_mode(hal::StatusLedMode::NORMAL);
			}

			nLength = -1;
			return nullptr;
		} else {
			const auto *pDmx = Dmx::GetDmxAvailable(0);

			if (__builtin_expect((pDmx != nullptr), 0)) {
				const auto *pDmxStatistics = reinterpret_cast<const struct Data *>(pDmx);
				nLength = static_cast<int16_t>(pDmxStatistics->Statistics.nSlotsInPacket);

				++pDmx;

				m_pDmxNodeOutputType->SetData(0, pDmx, static_cast<uint16_t>(nLength));

				if (!m_IsActive) {
					m_pDmxNodeOutputType->Start(0);
					m_IsActive = true;
					hal::statusled_set_mode(hal::StatusLedMode::DATA);
				}

				return const_cast<uint8_t *>(pDmx);
			}
		}

		nLength = 0;
		return nullptr;
	}

	void SetDisableOutput(const bool bDisable = true) {
		m_bDisableOutput = bDisable;
	}

	uint32_t GetUpdatesPerSecond(const uint32_t nPortIndex) {
		return Dmx::GetDmxUpdatesPerSecond(nPortIndex);
	}

	const uint8_t *GetDmxCurrentData(const uint32_t nPortIndex) {
		return Dmx::GetDmxCurrentData(nPortIndex);
	}

	void Print() {
		printf(" Output %s\n", m_bDisableOutput ? "disabled" : "enabled");
	}

private:
	DmxNodeOutputType *m_pDmxNodeOutputType { nullptr };
	bool m_IsActive { false };
	bool m_bDisableOutput { false };
};

#endif /* DMXRECEIVER_H */
