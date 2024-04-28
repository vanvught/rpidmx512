/**
 * @file showfileparams.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SHOWFILEPARAMS_H_
#define SHOWFILEPARAMS_H_

#include <cstdint>

#include "showfile.h"
#include "configstore.h"

namespace showfileparams {
struct Params {
	uint32_t nSetList;
	uint8_t nShow;
	uint16_t nOscPortIncoming;
	uint16_t nOscPortOutgoing;
	uint16_t nUniverse;
	uint8_t nDisableUnicast;
	uint8_t nDmxMaster;
} __attribute__((packed));

struct Mask {
	static constexpr uint32_t SHOW = (1U << 0);
	static constexpr uint32_t OSC_PORT_INCOMING = (1U << 1);
	static constexpr uint32_t OSC_PORT_OUTGOING = (1U << 2);
	static constexpr uint32_t PROTOCOL = (1U << 3);
	static constexpr uint32_t SACN_UNIVERSE = (1U << 4);
	static constexpr uint32_t ARTNET_UNICAST_DISABLED = (1U << 5);
	static constexpr uint32_t DMX_MASTER = (1U << 6);
	static constexpr uint32_t OPTION_AUTO_PLAY = (1U << 7);
	static constexpr uint32_t OPTION_LOOP = (1U << 8);
	static constexpr uint32_t OPTION_DISABLE_SYNC = (1U << 9);
};
}  // namespace showfileparams

class ShowFileParamsStore {
public:
	static void Update(const struct showfileparams::Params *ptShowFileParams) {
		ConfigStore::Get()->Update(configstore::Store::SHOW, ptShowFileParams, sizeof(struct showfileparams::Params));
	}

	static void Copy(struct showfileparams::Params *ptShowFileParams) {
		ConfigStore::Get()->Copy(configstore::Store::SHOW, ptShowFileParams, sizeof(struct showfileparams::Params));
	}
};

class ShowFileParams {
public:
	ShowFileParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TShowFileParams *ptShowFileParamss, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

	bool IsArtNetBroadcast() const {
		return isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED);
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void HandleOptions(const char *pLine, const char *pKeyword, uint16_t nMask);
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }
    void SetBool(const uint8_t nValue, const uint32_t nMask);

private:
    showfileparams::Params m_Params;
};

#endif /* SHOWFILEPARAMS_H_ */
