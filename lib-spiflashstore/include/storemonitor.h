/**
 * @file storemonitor.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STOREMONITOR_H_
#define STOREMONITOR_H_

#include "dmxmonitorparams.h"
#include "dmxmonitorstore.h"

#include "spiflashstore.h"

#include "debug.h"

class StoreMonitor final: public DMXMonitorParamsStore, public DmxMonitorStore {
public:
	StoreMonitor();

	void Update(const struct TDMXMonitorParams *pDMXMonitorParams) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Update(spiflashstore::Store::MONITOR, pDMXMonitorParams, sizeof(struct TDMXMonitorParams));

		DEBUG_EXIT
	}

	void Copy(struct TDMXMonitorParams *pDMXMonitorParams) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Copy(spiflashstore::Store::MONITOR, pDMXMonitorParams, sizeof(struct TDMXMonitorParams));

		DEBUG_EXIT
	}

	void SaveDmxStartAddress(uint16_t nDmxStartAddress) override {
		DEBUG_ENTRY

		SpiFlashStore::Get()->Update(spiflashstore::Store::MONITOR, __builtin_offsetof(struct TDMXMonitorParams, nDmxStartAddress), &nDmxStartAddress, sizeof(uint16_t), DMXMonitorParamsMask::START_ADDRESS);

		DEBUG_EXIT
	}

	static StoreMonitor* Get() {
		return s_pThis;
	}

private:
	static StoreMonitor *s_pThis;
};

#endif /* STOREMONITOR_H_ */
