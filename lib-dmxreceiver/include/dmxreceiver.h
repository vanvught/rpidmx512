/**
 * @file dmxreceiver.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "lightset.h"
#include "hardware.h"

#include "debug.h"

class DMXReceiver: Dmx {
public:
	DMXReceiver(LightSet *pLightSet) {
		s_pLightSet = pLightSet;
	}

	~DMXReceiver() {
		DMXReceiver::Stop();
		s_IsActive = false;
	}

	void Start() {
		Dmx::SetPortDirection(0, dmx::PortDirection::INP, true);
	}

	void Stop() {
		Dmx::SetPortDirection(0, dmx::PortDirection::INP, false);
		s_pLightSet->Stop(0);
	}

	void SetLightSet(LightSet *pLightSet) {
		if (pLightSet != s_pLightSet) {
			s_pLightSet->Stop(0);
			s_pLightSet = pLightSet;
			s_IsActive = false;
		}

	}

	const uint8_t* Run(int16_t &nLength) {
		if (__builtin_expect((s_bDisableOutput), 0)) {
			nLength = 0;
			return nullptr;
		}

		if (Dmx::GetUpdatesPerSecond(0) == 0) {
			if (s_IsActive) {
				s_pLightSet->Stop(0);
				s_IsActive = false;
				Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			}

			nLength = -1;
			return nullptr;
		} else {
			const auto *pDmx = Dmx::GetDmxAvailable(0);

			if (pDmx != nullptr) {
				const auto *pDmxStatistics = reinterpret_cast<const struct Data*>(pDmx);
				nLength = static_cast<int16_t>(pDmxStatistics->Statistics.nSlotsInPacket);

				++pDmx;

				s_pLightSet->SetData(0, pDmx, static_cast<uint16_t>(nLength));

				if (!s_IsActive) {
					s_pLightSet->Start(0);
					s_IsActive = true;
					Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
				}

				return const_cast<uint8_t*>(pDmx);
			}
		}

		nLength = 0;
		return nullptr;
	}

	void SetDisableOutput(bool bDisable = true) {
		s_bDisableOutput = bDisable;
	}

	uint32_t GetUpdatesPerSecond(uint32_t nPortIndex) {
		return Dmx::GetUpdatesPerSecond(nPortIndex);
	}

	const uint8_t* GetDmxCurrentData(uint32_t nPortIndex) {
		return Dmx::GetDmxCurrentData(nPortIndex);
	}

	void Print() {
		printf(" Output %s\n", s_bDisableOutput ? "disabled" : "enabled");
	}

private:
	static LightSet *s_pLightSet;
	static bool s_IsActive;
	static bool s_bDisableOutput;
};

#endif /* DMXRECEIVER_H */
