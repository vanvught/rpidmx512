/**
 * @file artnetparams.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETPARAMS_H_
#define ARTNETPARAMS_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "artnetnode.h"

#include "lightset.h"

struct TArtNetParams {
	uint32_t nSetList;
	uint8_t nNet;
	uint8_t nSubnet;
	uint8_t nUniverse;
	TLightSetOutputType tOutputType;
	bool bUseTimeCode;
	bool bUseTimeSync;
	bool bEnableRdm;
	bool bRdmDiscovery;
	uint8_t aShortName[ARTNET_SHORT_NAME_LENGTH];
	uint8_t aLongName[ARTNET_LONG_NAME_LENGTH];
	uint8_t filler[2]; // aManufacturerId
	uint8_t aOemValue[2];
	time_t nNetworkTimeout;
	bool bDisableMergeTimeout;
	uint8_t nUniversePort[ARTNET_MAX_PORTS];
	uint8_t nMergeMode;
	uint8_t nMergeModePort[ARTNET_MAX_PORTS];
	uint8_t nProtocol;
	uint8_t nProtocolPort[ARTNET_MAX_PORTS];
	bool bEnableNoChangeUpdate;
	uint8_t nDirection;
	uint32_t nDestinationIp;
};

enum TArtnetParamsMask {
	ARTNET_PARAMS_MASK_LONG_NAME = (1 << 0),
	ARTNET_PARAMS_MASK_SHORT_NAME = (1 << 1),
	ARTNET_PARAMS_MASK_NET = (1 << 2),
	ARTNET_PARAMS_MASK_SUBNET = (1 << 3),
	ARTNET_PARAMS_MASK_UNIVERSE = (1 << 4),
	ARTNET_PARAMS_MASK_RDM = (1 << 5),
	ARTNET_PARAMS_MASK_TIMECODE = (1 << 6),
	ARTNET_PARAMS_MASK_TIMESYNC = (1 << 7),
	ARTNET_PARAMS_MASK_OUTPUT = (1 << 8),
	//ARTNET_PARAMS_MASK_NOT_USED = (1 << 9), // aManufacturerId
	ARTNET_PARAMS_MASK_OEM_VALUE = (1 << 10),
	ARTNET_PARAMS_MASK_NETWORK_TIMEOUT = (1 << 11),
	ARTNET_PARAMS_MASK_MERGE_TIMEOUT = (1 << 12),
	ARTNET_PARAMS_MASK_UNIVERSE_A = (1 << 13),
	ARTNET_PARAMS_MASK_UNIVERSE_B = (1 << 14),
	ARTNET_PARAMS_MASK_UNIVERSE_C = (1 << 15),
	ARTNET_PARAMS_MASK_UNIVERSE_D = (1 << 16),
	ARTNET_PARAMS_MASK_MERGE_MODE = (1 << 17),
	ARTNET_PARAMS_MASK_MERGE_MODE_A = (1 << 18),
	ARTNET_PARAMS_MASK_MERGE_MODE_B = (1 << 19),
	ARTNET_PARAMS_MASK_MERGE_MODE_C = (1 << 20),
	ARTNET_PARAMS_MASK_MERGE_MODE_D = (1 << 21),
	ARTNET_PARAMS_MASK_PROTOCOL = (1 << 22),
	ARTNET_PARAMS_MASK_PROTOCOL_A = (1 << 23),
	ARTNET_PARAMS_MASK_PROTOCOL_B = (1 << 24),
	ARTNET_PARAMS_MASK_PROTOCOL_C = (1 << 25),
	ARTNET_PARAMS_MASK_PROTOCOL_D = (1 << 26),
	ARTNET_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT = (1 << 27),
	ARTNET_PARAMS_MASK_DIRECTION = (1 << 28),
	ARTNET_PARAMS_MASK_DESTINATION_IP = (1 << 29)
};

class ArtNetParamsStore {
public:
	virtual ~ArtNetParamsStore(void) {}

	virtual void Update(const struct TArtNetParams *pArtNetParams)=0;
	virtual void Copy(struct TArtNetParams *pArtNetParams)=0;
};

class ArtNetParams {
public:
	ArtNetParams(ArtNetParamsStore *pArtNetParamsStore = 0);
	~ArtNetParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TArtNetParams *pArtNetParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(ArtNetNode *);

	void Dump(void);

	uint8_t GetNet(void) {
		return m_tArtNetParams.nNet;
	}

	uint8_t GetSubnet(void) {
		return m_tArtNetParams.nSubnet;
	}

	uint8_t GetUniverse(void) {
		return m_tArtNetParams.nUniverse;
	}

	const uint8_t *GetShortName(void) {
		return m_tArtNetParams.aShortName;
	}

	const uint8_t *GetLongName(void) {
		return m_tArtNetParams.aLongName;
	}

	TLightSetOutputType GetOutputType(void) {
		return m_tArtNetParams.tOutputType;
	}

	time_t GetNetworkTimeout(void) {
		return m_tArtNetParams.nNetworkTimeout;
	}

	bool IsUseTimeCode(void) {
		return m_tArtNetParams.bUseTimeCode;
	}

	bool IsUseTimeSync(void) {
		return m_tArtNetParams.bUseTimeSync;
	}

	bool IsRdm(void) {
		return m_tArtNetParams.bEnableRdm;
	}

	bool IsRdmDiscovery(void) {
		return m_tArtNetParams.bRdmDiscovery;
	}

	uint8_t GetUniverse(uint8_t nPort, bool &IsSet);

	bool IsEnableNoChangeUpdate(void) {
		return m_tArtNetParams.bEnableNoChangeUpdate;
	}

	TArtNetPortDir GetDirection(void) {
		return (TArtNetPortDir) m_tArtNetParams.nDirection;
	}

	uint32_t GetDestinationIp(void) {
		return m_tArtNetParams.nDestinationIp;
	}

public:
	static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) {
		return (m_tArtNetParams.nSetList & nMask) == nMask;
	}

private:
	ArtNetParamsStore *m_pArtNetParamsStore;
	struct TArtNetParams m_tArtNetParams;
};

#endif /* ARTNETPARAMS_H_ */
