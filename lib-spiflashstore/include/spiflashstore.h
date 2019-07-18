/**
 * @file spiflashstore.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "storedmxsend.h"
#include "storews28xxdmx.h"
#include "storetlc59711.h"
#include "storee131.h"
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

	void Update(enum TStore tStore, uint32_t nOffset, void *pData, uint32_t nDataLength, uint32_t bSetList = 0);
	void Update(enum TStore tStore, void *pData, uint32_t nDataLength) {
		Update(tStore, 0, pData, nDataLength);
	}
	void Copy(enum TStore tStore, void *pData, uint32_t nDataLength);

	void CopyTo(enum TStore tStore, void *pData, uint32_t& nDataLength);

	bool Flash(void);

	void Dump(void);

	StoreNetwork *GetStoreNetwork(void);
	StoreArtNet *GetStoreArtNet(void);
	StoreDmxSend *GetStoreDmxSend(void);
	StoreWS28xxDmx *GetStoreWS28xxDmx(void);
	StoreE131 *GetStoreE131(void);
	StoreArtNet4 *GetStoreArtNet4(void);
	StoreTLC59711 *GetStoreTLC59711(void);

private:
	bool Init(void);
	uint32_t GetStoreOffset(enum TStore tStore);

public:
	static SpiFlashStore* Get(void) {
		return s_pThis;
	}

private:
	static SpiFlashStore *s_pThis;

private:
	bool m_bHaveFlashChip;
	bool m_bIsNew;
	uint32_t m_nStartAddress;
	alignas(uint32_t) uint8_t m_aSpiFlashData[SPI_FLASH_STORE_SIZE];
	uint32_t m_nSpiFlashStoreSize;
	TStoreState m_tState;

	StoreNetwork m_StoreNetwork;
	StoreArtNet m_StoreArtNet;
	StoreDmxSend m_StoreDmxSend;
	StoreWS28xxDmx m_StoreWS28xxDmx;
	StoreE131 m_StoreE131;
	StoreArtNet4 m_StoreArtNet4;
	StoreTLC59711 m_StoreTLC59711;
};

#endif /* SPIFLASHSTORE_H_ */
