/**
 * @file spiflashstore.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "storenetwork.h"

#define SPI_FLASH_STORE_SIZE	4096

enum TStore {
	STORE_NETWORK,
	STORE_ARTNET,
	STORE_DMXSEND,
	STORE_WS28XXDMX,
	STORE_E131,
	STORE_LTC,
	STORE_MIDI,
	STORE_ARTNET4,
	STORE_OSC,
	STORE_TLC5711DMX,
	STORE_WIDGET,
	STORE_RDMDEVICE,
	STORE_RCONFIG,
	STORE_TCNET,
	STORE_OSC_CLIENT,
	STORE_DISPLAYUDF,
	STORE_LTCDISPLAY,
	STORE_MONITOR,
	STORE_SPARKFUN,
	STORE_SLUSH,
	STORE_MOTORS,
	STORE_SHOW,
	STORE_SERIAL,
	STORE_RDMSENSORS,
	STORE_RDMSUBDEVICES,
	STORE_GPS,
	STORE_RGBPANEL,
	STORE_LAST
};

enum TStoreState {
	STORE_STATE_IDLE,
	STORE_STATE_CHANGED,
	STORE_STATE_ERASED
};

class SpiFlashStore {
public:
	SpiFlashStore();
	~SpiFlashStore();

	 bool HaveFlashChip() {
		return m_bHaveFlashChip;
	}

	void Update(TStore tStore, uint32_t nOffset, const void *pData, uint32_t nDataLength, uint32_t nSetList = 0, uint32_t nOffsetSetList = 0);
	void Update(TStore tStore, const void *pData, uint32_t nDataLength) {
		Update(tStore, 0, pData, nDataLength);
	}
	void Copy(TStore tStore, void *pData, uint32_t nDataLength, uint32_t nOffset = 0);
	void CopyTo(TStore tStore, void *pData, uint32_t &nDataLength);

	void ResetSetList(TStore tStore);

	bool Flash();

	void Dump();

	static SpiFlashStore *Get() {
		return s_pThis;
	}

private:
	bool Init();
	uint32_t GetStoreOffset(TStore tStore);

private:
	bool m_bHaveFlashChip{false};
	bool m_bIsNew{false};
	uint32_t m_nStartAddress{0};
	uint32_t m_nSpiFlashStoreSize{SPI_FLASH_STORE_SIZE};
	TStoreState m_tState{STORE_STATE_IDLE};
	uint8_t m_aSpiFlashData[SPI_FLASH_STORE_SIZE];

	StoreNetwork m_StoreNetwork;

	static SpiFlashStore *s_pThis;
};

#endif /* SPIFLASHSTORE_H_ */
