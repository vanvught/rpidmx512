/**
 * @file dmxmonitorparams.h
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

#ifndef DMXMONITORPARAMS_H_
#define DMXMONITORPARAMS_H_

#include <cstdint>

#include "dmxmonitor.h"
#include "configstore.h"

struct TDMXMonitorParams {
	uint32_t nSetList;
	uint16_t nDmxStartAddress;
	uint16_t nDmxMaxChannels;
	uint8_t tFormat;
} __attribute__((packed));

struct DMXMonitorParamsMask {
	static constexpr uint32_t START_ADDRESS = (1U << 0);
	static constexpr uint32_t MAX_CHANNELS = (1U << 1);
	static constexpr uint32_t FORMAT = (1U << 2);
};

class DmxMonitorParamsStore {
public:
	static void Update(const struct TDMXMonitorParams *pParams) {
		ConfigStore::Get()->Update(configstore::Store::MONITOR, pParams, sizeof(struct TDMXMonitorParams));
	}

	static void Copy(struct TDMXMonitorParams *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::MONITOR, pParams, sizeof(struct TDMXMonitorParams));
	}
};

class DMXMonitorParams {
public:
	DMXMonitorParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDMXMonitorParams *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}
	
	void Set(DMXMonitor *pDMXMonitor);

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    struct TDMXMonitorParams m_Params;
};

#endif /* DMXMONITORPARAMS_H_ */
