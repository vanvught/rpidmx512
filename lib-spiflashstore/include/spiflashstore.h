/**
 * @file spiflashstore.h
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SPIFLASHSTORE_H_
#define SPIFLASHSTORE_H_

#include <cstdint>

namespace spiflashstore {
enum class Store {
	NETWORK,
	ARTNET,
	DMXSEND,
	WS28XXDMX,
	E131,
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

}  // namespace spiflashstore

class SpiFlashStore {
public:
	SpiFlashStore();
	~SpiFlashStore() {
		while (Flash())
			;
	}

	 bool HaveFlashChip() const {
		return s_bHaveFlashChip;
	}

	void Update(spiflashstore::Store tStore, uint32_t nOffset, const void *pData, uint32_t nDataLength, uint32_t nSetList = 0, uint32_t nOffsetSetList = 0);
	void Update(spiflashstore::Store tStore, const void *pData, uint32_t nDataLength) {
		Update(tStore, 0, pData, nDataLength);
	}
	void Copy(spiflashstore::Store tStore, void *pData, uint32_t nDataLength, uint32_t nOffset = 0, bool isSetList = true);
	void CopyTo(spiflashstore::Store tStore, void *pData, uint32_t& nDataLength);

	void ResetSetList(spiflashstore::Store tStore);

	bool Flash();

	void Dump();

	static SpiFlashStore *Get() {
		return s_pThis;
	}

private:
	bool Init();
	uint32_t GetStoreOffset(spiflashstore::Store tStore);

private:
	struct FlashStore {
		static constexpr auto SIZE = 4096U;
	};

	static bool s_bHaveFlashChip;
	static bool s_bIsNew;

	static spiflashstore::State s_State;
	static uint32_t s_nStartAddress;

	static uint32_t s_nSpiFlashStoreSize;
	static uint8_t s_SpiFlashData[FlashStore::SIZE];

	static uint32_t s_nWaitMillis;

	static SpiFlashStore *s_pThis;
};

#endif /* SPIFLASHSTORE_H_ */
