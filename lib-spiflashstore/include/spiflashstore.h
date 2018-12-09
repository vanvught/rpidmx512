/**
 * @file spiflashstore.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LIB_SPIFLASHSTORE_H_
#define LIB_SPIFLASHSTORE_H_

#include <stdint.h>
#include <uuid/uuid.h>

#include "storenetwork.h"
#include "storeartnet.h"
#if defined(BARE_METAL)
 #include "storedmxsend.h"
 #include "storews28xxdmx.h"
#endif
#include "storee131.h"

#define SPI_FLASH_STORE_SIZE	4096

enum TStore {
	STORE_NETWORK,
	STORE_ARTNET,
	STORE_DMXSEND,
	STORE_WS28XXDMX,
	STORE_E131,
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

	inline bool HaveFlashChip(void) {
		return m_bHaveFlashChip;
	}

	void Update(enum TStore tStore, uint32_t nOffset, void *pData, uint32_t nDataLength, uint32_t bSetList = 0);
	void Update(enum TStore tStore, void *pData, uint32_t nDataLength) {
		Update(tStore, 0, pData, nDataLength);
	}
	void Copy(enum TStore tStore, void *pData, uint32_t nDataLength);

	void UuidUpdate(const uuid_t uuid);
	void UuidCopyTo(uuid_t uuid);

	bool Flash(void);

	void Dump(void);

	inline StoreNetwork *GetStoreNetwork(void) {
		return &m_StoreNetwork;
	}
	inline StoreArtNet *GetStoreArtNet(void) {
		return &m_StoreArtNet;
	}
#if defined(BARE_METAL)
	inline StoreDmxSend *GetStoreDmxSend(void) {
		return &m_StoreDmxSend;
	}
	inline StoreWS28xxDmx *GetStoreWS28xxDmx(void) {
		return &m_StoreWS28xxDmx;
	}
#endif
	inline StoreE131 *GetStoreE131(void) {
		return &m_StoreE131;
	}

private:
	bool Init(void);
	uint32_t GetStoreOffset(enum TStore tStore);

public:
	inline static SpiFlashStore* Get(void) { return s_pThis; }

private:
	static SpiFlashStore *s_pThis;

private:
	bool m_bHaveFlashChip;
	bool m_bIsNew;
	uint32_t m_nStartAddress;
	uint8_t m_aSpiFlashData[SPI_FLASH_STORE_SIZE];
	uint32_t m_nSpiFlashStoreSize;
	TStoreState m_tState;

	StoreNetwork m_StoreNetwork;
	StoreArtNet m_StoreArtNet;
#if defined(BARE_METAL)
	StoreDmxSend m_StoreDmxSend;
	StoreWS28xxDmx m_StoreWS28xxDmx;
#endif
	StoreE131 m_StoreE131;
};

#endif /* LIB_SPIFLASHSTORE_H_ */
