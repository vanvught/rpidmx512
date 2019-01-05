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

#include "networkparams.h"

class StoreNetwork: public NetworkParamsStore {
public:
	StoreNetwork(void);
	~StoreNetwork(void);

	void Update(const struct TNetworkParams *pNetworkParams);
	void Copy(struct TNetworkParams *pNetworkParams);

	void UpdateIp(uint32_t nIp);
	void UpdateNetMask(uint32_t nNetMask);

private:

};

#if defined (LIB_SPIFLASHSTORE) || defined (ARTNET_NODE) || defined (LTC_READER)
	#include "artnetparams.h"
	#include "artnetstore.h"

	class StoreArtNet: public ArtNetParamsStore, ArtNetStore {
	public:
		StoreArtNet(void);
		~StoreArtNet(void);

		void Update(const struct TArtNetParams *pArtNetParams);
		void Copy(struct TArtNetParams *pArtNetParams);

		void SaveShortName(const char *pShortName);
		void SaveLongName(const char *pLongName);

		void SaveUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress);
		void SaveNetSwitch(uint8_t nAddress);
		void SaveSubnetSwitch(uint8_t nAddress);

		void SaveMergeMode(uint8_t nPortIndex, TMerge tMerge);

		void SavePortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol);

	private:
	};
#else
	class StoreArtNet {
	public:
		StoreArtNet(void);
		~StoreArtNet(void);
	};
#endif

#if defined (LIB_SPIFLASHSTORE) || defined (ARTNET_NODE) || defined (E131_BRIDGE)
	#include "dmxparams.h"

	class StoreDmxSend: public DMXParamsStore {
	public:
		StoreDmxSend(void);
		~StoreDmxSend(void);

		void Update(const struct TDMXParams *pDMXParams);
		void Copy(struct TDMXParams *pDMXParams);

	private:

	};
#else
	class StoreDmxSend {
	public:
		StoreDmxSend(void);
		~StoreDmxSend(void);
	};
#endif

#if defined (LIB_SPIFLASHSTORE) || (defined (ARTNET_NODE) && !defined(LTC_READER))
	#include "ws28xxdmxparams.h"

	class StoreWS28xxDmx: public WS28xxDmxParamsStore {
	public:
		StoreWS28xxDmx(void);
		~StoreWS28xxDmx(void);

		void Update(const struct TWS28xxDmxParams *pWS28xxDmxParams);
		void Copy(struct TWS28xxDmxParams *pWS28xxDmxParams);

	private:

	};
#else
	class StoreWS28xxDmx {
	public:
		StoreWS28xxDmx(void);
		~StoreWS28xxDmx(void);
	};
#endif

#if defined (LIB_SPIFLASHSTORE) || defined (E131_BRIDGE)
	#include "e131params.h"

	class StoreE131: public E131ParamsStore {
	public:
		StoreE131(void);
		~StoreE131(void);

		void Update(const struct TE131Params *pE131Params);
		void Copy(struct TE131Params *pE131Params);

	private:

	};
#else
	class StoreE131{
	public:
		StoreE131(void);
		~StoreE131(void);
	};
#endif

#if defined (LIB_SPIFLASHSTORE) || defined (LTC_READER)
	#include "ltcparams.h"

	class StoreLtc: public LtcParamsStore {
	public:
		StoreLtc(void);
		~StoreLtc(void);

		void Update(const struct TLtcParams *pLtcParams);
		void Copy(struct TLtcParams *pLtcParams);
	private:
	};
#else
	class StoreLtc {
	public:
		StoreLtc(void);
		~StoreLtc(void);
	};
#endif

#if defined (LIB_SPIFLASHSTORE) || defined (LTC_READER)
	#include "midiparams.h"

	class StoreMidi: public MidiParamsStore {
	public:
		StoreMidi(void);
		~StoreMidi(void);

		void Update(const struct TMidiParams *pMidiParams);
		void Copy(struct TMidiParams *pMidiParams);
	private:
	};
#else
	class StoreMidi {
	public:
		StoreMidi(void);
		~StoreMidi(void);
	};
#endif

#define SPI_FLASH_STORE_SIZE	4096

enum TStore {
	STORE_NETWORK,
	STORE_ARTNET,
	STORE_DMXSEND,
	STORE_WS28XXDMX,
	STORE_E131,
	STORE_LTC,
	STORE_MIDI,
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

	StoreNetwork *GetStoreNetwork(void);
	StoreArtNet *GetStoreArtNet(void);
	StoreDmxSend *GetStoreDmxSend(void);
	StoreWS28xxDmx *GetStoreWS28xxDmx(void);
	StoreE131 *GetStoreE131(void);
	StoreLtc *GetStoreLtc(void);
	StoreMidi *GetStoreMidi(void);

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
	StoreDmxSend m_StoreDmxSend;
	StoreWS28xxDmx m_StoreWS28xxDmx;
	StoreE131 m_StoreE131;
	StoreLtc m_StoreLtc;
	StoreMidi m_StoreMidi;
};

#endif /* SPIFLASHSTORE_H_ */
