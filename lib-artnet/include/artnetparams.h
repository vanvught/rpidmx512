/**
 * @file artnetparams.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"); to deal
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

#include <cstdint>
#include <climits>
#include <cassert>

#include "artnetnode.h"

#include "lightset.h"

namespace artnetparams {
struct Params {
	uint32_t nSetList;									///< 4	  4
	uint8_t nNet;										///< 1	  5
	uint8_t nSubnet;									///< 1	  6
	uint8_t NotUsed9;									///< 1	  7
	uint8_t nOutputType;								///< 1	  8
	uint8_t NotUsed0;									///< 1	  9
	uint8_t NotUsed1;									///< 1	 10
	uint8_t NotUsed2;									///< 1	 11
	uint8_t NotUsed3;									///< 1	 12
	uint8_t aShortName[ArtNet::SHORT_NAME_LENGTH];		///< 18	 30
	uint8_t aLongName[ArtNet::LONG_NAME_LENGTH];		///< 64	 94
	uint16_t nMultiPortOptions;							///< 2	 96
	uint8_t aOemValue[2];								///< 2	 98
	uint8_t nNetworkTimeout;							///< 1	 99
	uint8_t NotUsed4;									///< 1	100
	uint8_t NotUsed5;									///< 1	101
	uint8_t NotUsed6;									///< 1	102
	uint8_t NotUsed7;									///< 1	103
	uint8_t nUniversePort[ArtNet::PORTS];				///< 4	107
	uint8_t nMergeMode;									///< 1	108
	uint8_t nMergeModePort[ArtNet::PORTS];				///< 4	112
	uint8_t nProtocol;									///< 1	113
	uint8_t nProtocolPort[ArtNet::PORTS];				///< 4	117
	uint8_t NotUsed8;									///< 1	118
	uint8_t nDirection;									///< 1	119
	uint32_t nDestinationIpPort[ArtNet::PORTS];			///< 16	135
}__attribute__((packed));

static_assert(sizeof(struct Params) <= 144, "struct Params is too large");

struct MaskMultiPortOptions {
	static constexpr auto DESTINATION_IP_A = (1U << 0);
	static constexpr auto DESTINATION_IP_B = (1U << 1);
	static constexpr auto DESTINATION_IP_C = (1U << 2);
	static constexpr auto DESTINATION_IP_D = (1U << 3);
};

struct Mask {
	static constexpr auto LONG_NAME = (1U << 0);
	static constexpr auto SHORT_NAME = (1U << 1);
	static constexpr auto NET = (1U << 2);
	static constexpr auto SUBNET = (1U << 3);
	//static constexpr auto UNIVERSE = (1U << 4);
	static constexpr auto RDM = (1U << 5);
	//static constexpr auto TIMECODE = (1U << 6);
	//static constexpr auto TIMESYNC = (1U << 7);
	static constexpr auto OUTPUT = (1U << 8);
	//static constexpr auto NOT_USED1 = (1U << 9);
	static constexpr auto OEM_VALUE = (1U << 10);
	static constexpr auto NETWORK_TIMEOUT = (1U << 11);
	static constexpr auto DISABLE_MERGE_TIMEOUT = (1U << 12);
	static constexpr auto UNIVERSE_A = (1U << 13);
	static constexpr auto UNIVERSE_B = (1U << 14);
	static constexpr auto UNIVERSE_C = (1U << 15);
	static constexpr auto UNIVERSE_D = (1U << 16);
	static constexpr auto MERGE_MODE = (1U << 17);
	static constexpr auto MERGE_MODE_A = (1U << 18);
	static constexpr auto MERGE_MODE_B = (1U << 19);
	static constexpr auto MERGE_MODE_C = (1U << 20);
	static constexpr auto MERGE_MODE_D = (1U << 21);
	static constexpr auto PROTOCOL = (1U << 22);
	static constexpr auto PROTOCOL_A = (1U << 23);
	static constexpr auto PROTOCOL_B = (1U << 24);
	static constexpr auto PROTOCOL_C = (1U << 25);
	static constexpr auto PROTOCOL_D = (1U << 26);
	static constexpr auto MAP_UNIVERSE0 = (1U << 27);
	//static constexpr auto DIRECTION = (1U << 28);
};

}  // namespace artnetparams

class ArtNetParamsStore {
public:
	virtual ~ArtNetParamsStore() {}

	virtual void Update(const struct artnetparams::Params *pArtNetParams)=0;
	virtual void Copy(struct artnetparams::Params *pArtNetParams)=0;
};

class ArtNetParams {
public:
	ArtNetParams(ArtNetParamsStore *pArtNetParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct artnetparams::Params *pArtNetParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(ArtNetNode *);

	void Dump();

	uint8_t GetNet() const {
		return m_tArtNetParams.nNet;
	}

	uint8_t GetSubnet() const {
		return m_tArtNetParams.nSubnet;
	}

	const uint8_t *GetShortName() const {
		return m_tArtNetParams.aShortName;
	}

	const uint8_t *GetLongName() const {
		return m_tArtNetParams.aLongName;
	}

	lightset::OutputType GetOutputType() const {
		return static_cast<lightset::OutputType>(m_tArtNetParams.nOutputType);
	}

	uint32_t GetNetworkTimeout() const {
		return m_tArtNetParams.nNetworkTimeout;
	}

	bool IsRdm() const {
		return isMaskSet(artnetparams::Mask::RDM);
	}

	uint8_t GetUniverse(uint8_t nPortIndex, bool &IsSet) const {
		if (nPortIndex < ArtNet::PORTS) {
			IsSet = isMaskSet(artnetparams::Mask::UNIVERSE_A << nPortIndex);
			return m_tArtNetParams.nUniversePort[nPortIndex];
		}
		IsSet = false;
		return 0;
	}


	lightset::PortDir GetDirection(uint32_t nPortIndex) const {
		if (nPortIndex < CHAR_BIT) {
			return static_cast<lightset::PortDir>((m_tArtNetParams.nDirection >> nPortIndex) & 0x1);
		}
		return lightset::PortDir::DISABLE;
	}

	static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	void SetBool(const uint8_t nValue, const uint32_t nMask);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tArtNetParams.nSetList & nMask) == nMask;
	}
	bool isMaskMultiPortOptionsSet(uint16_t nMask) const {
		return (m_tArtNetParams.nMultiPortOptions & nMask) == nMask;
	}

private:
	ArtNetParamsStore *m_pArtNetParamsStore;
	artnetparams::Params m_tArtNetParams;
};

#endif /* ARTNETPARAMS_H_ */
