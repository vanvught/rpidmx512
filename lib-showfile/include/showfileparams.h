/**
 * @file showfileparams.h
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

namespace showfileparams {
struct Params {
	uint32_t nSetList;
	uint8_t nFormat;
	uint8_t nShow;
	uint16_t nOptions;
	uint16_t nOscPortIncoming;
	uint16_t nOscPortOutgoing;
	uint8_t nProtocol;
	uint16_t nUniverse;
	uint8_t nDisableUnicast;
	uint8_t nDmxMaster;
} __attribute__((packed));

struct Options {
	static constexpr auto AUTO_START = (1U << 0);
	static constexpr auto LOOP = (1U << 1);
	static constexpr auto DISABLE_SYNC = (1U << 2);
};

struct Mask {
	static constexpr auto FORMAT = (1U << 0);
	static constexpr auto SHOW = (1U << 1);
	static constexpr auto OPTIONS = (1U << 2);
	static constexpr auto OSC_PORT_INCOMING = (1U << 3);
	static constexpr auto OSC_PORT_OUTGOING = (1U << 4);
	static constexpr auto PROTOCOL = (1U << 5);
	static constexpr auto SACN_UNIVERSE = (1U << 6);
	static constexpr auto ARTNET_UNICAST_DISABLED = (1U << 7);
	static constexpr auto DMX_MASTER = (1U << 8);
};
}  // namespace showfileparams

class ShowFileParamsStore {
public:
	virtual ~ShowFileParamsStore() {}

	virtual void Update(const struct showfileparams::Params *pShowFileParams)=0;
	virtual void Copy(struct showfileparams::Params *pShowFileParams)=0;
};

class ShowFileParams {
public:
	ShowFileParams(ShowFileParamsStore *pShowFileParamsStore);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TShowFileParams *ptShowFileParamss, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

	void Dump();

	showfile::Formats GetFormat() const {
		return static_cast<showfile::Formats>(m_showFileParams.nFormat);
	}

	showfile::Protocols GetProtocol() const {
		return static_cast<showfile::Protocols>(m_showFileParams.nProtocol);
	}

	uint8_t GetShow() const {
		return m_showFileParams.nShow;
	}

	bool IsAutoStart() const {
		return isOptionSet(showfileparams::Options::AUTO_START);
	}

	bool IsArtNetBroadcast() const {
		return isMaskSet(showfileparams::Mask::ARTNET_UNICAST_DISABLED);
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
    void HandleOptions(const char *pLine, const char *pKeyword, uint16_t nMask);
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_showFileParams.nSetList & nMask) == nMask;
    }
    bool isOptionSet(uint16_t nMask) const {
    	return (m_showFileParams.nOptions & nMask) == nMask;
    }

private:
    ShowFileParamsStore *m_pShowFileParamsStore;
    showfileparams::Params m_showFileParams;
};

#endif /* SHOWFILEPARAMS_H_ */
