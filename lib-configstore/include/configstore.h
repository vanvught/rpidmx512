/**
 * @file configstore.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef CONFIGSTORE_H_
#define CONFIGSTORE_H_

#include <cstdint>

#include "storedevice.h"

namespace configstore {
enum class Store {
	NETWORK,
	DMXSEND,
	WS28XXDMX,
	LTC,
	MIDI,
	LTCETC,
	OSC,
	TLC5711DMX,
	WIDGET,
	RDMDEVICE,
	RCONFIG,
	TCNET,
	OSC_CLIENT,
	DISPLAYUDF,
	LTCDISPLAY,
	MONITOR,
	SPARKFUN,
	SLUSH,
	MOTORS,
	SHOW,
	SERIAL,
	RDMSENSORS,
	RDMSUBDEVICES,
	GPS,
	RGBPANEL,
	NODE,
	LAST
};

enum class State {
	IDLE, CHANGED, CHANGED_WAITING, ERASING, ERASED, ERASED_WAITING, WRITING
};

}  // namespace configstore

class ConfigStore: StoreDevice {
public:
	ConfigStore();
	~ConfigStore() {
		while (Flash())
			;
	}

	 bool HaveFlashChip() const {
		return s_bHaveFlashChip;
	}

	void Update(configstore::Store store, uint32_t nOffset, const void *pData, uint32_t nDataLength, uint32_t nSetList = 0, uint32_t nOffsetSetList = 0);
	void Update(configstore::Store store, const void *pData, uint32_t nDataLength) {
		Update(store, 0, pData, nDataLength);
	}

	void Copy(const configstore::Store store, void *pData, uint32_t nDataLength, uint32_t nOffset = 0);

	void ResetSetList(configstore::Store store);

	bool Flash();

	void Dump();

	void Delay();

	static ConfigStore *Get() {
		return s_pThis;
	}

private:
	uint32_t GetStoreOffset(configstore::Store tStore);

private:
	struct FlashStore {
		static constexpr auto SIZE = 4096U;
	};

	static bool s_bHaveFlashChip;

	static configstore::State s_State;
	static uint32_t s_nStartAddress;

	static uint32_t s_nSpiFlashStoreSize;
	static uint8_t s_SpiFlashData[FlashStore::SIZE];

	static uint32_t s_nWaitMillis;

	static ConfigStore *s_pThis;
};

#endif /* CONFIGSTORE_H_ */
