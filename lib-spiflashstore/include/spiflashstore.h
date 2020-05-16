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
#include <uuid/uuid.h>

#include "storenetwork.h"
#include "storeartnet.h"
#include "storeartnet4.h"

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
	STORE_LAST
};

enum TStoreState {
	STORE_STATE_IDLE,
	STORE_STATE_CHANGED,
	STORE_STATE_ERASED
};

class SpiFlashStore {
public:
	SpiFlashStore(void);
	~SpiFlashStore(void);

	 bool HaveFlashChip(void) {
		return m_bHaveFlashChip;
	}

	void Update(enum TStore tStore, uint32_t nOffset, const void *pData, uint32_t nDataLength, uint32_t nSetList = 0, uint32_t nOffsetSetList = 0);
	void Update(enum TStore tStore, const void *pData, uint32_t nDataLength) {
		Update(tStore, 0, pData, nDataLength);
	}
	void Copy(enum TStore tStore, void *pData, uint32_t nDataLength, uint32_t nOffset = 0);
	void CopyTo(enum TStore tStore, void *pData, uint32_t &nDataLength);

	void ResetSetList(enum TStore tStore);

	bool Flash(void);

	void Dump(void);

	StoreNetwork *GetStoreNetwork(void);
	StoreArtNet *GetStoreArtNet(void);
	StoreArtNet4 *GetStoreArtNet4(void);

private:
	bool Init(void);
	uint32_t GetStoreOffset(enum TStore tStore);

public:
	static SpiFlashStore *Get(void) {
		return s_pThis;
	}

private:
	static SpiFlashStore *s_pThis;

private:
	bool m_bHaveFlashChip;
	bool m_bIsNew;
	uint32_t m_nStartAddress;
	uint32_t m_nSpiFlashStoreSize;
	TStoreState m_tState;

	alignas(uintptr_t) uint8_t m_aSpiFlashData[SPI_FLASH_STORE_SIZE];

	StoreNetwork m_StoreNetwork;
	StoreArtNet m_StoreArtNet;
	StoreArtNet4 m_StoreArtNet4;
};

#endif /* SPIFLASHSTORE_H_ */
