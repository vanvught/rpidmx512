/**
 * @file ltcetcparams.h
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCETCPARAMS_H_
#define LTCETCPARAMS_H_

#include <cstdint>

#include "configstore.h"

namespace ltcetcparams {
struct Params {
    uint32_t nSetList;				///< 4	 4
    uint32_t nDestinationIp;		///< 4	 8
    uint32_t nSourceMulticastIp;	///< 4	12
    uint16_t nDestinationPort;		///< 2	14
    uint16_t nSourcePort;			///< 2	16
    uint8_t nUdpTerminator;			///< 1	17
}__attribute__((packed));

static_assert(sizeof(struct ltcetcparams::Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr auto DESTINATION_IP = (1U << 0);
	static constexpr auto DESTINATION_PORT = (1U << 1);
	static constexpr auto SOURCE_MULTICAST_IP = (1U << 2);
	static constexpr auto SOURCE_PORT = (1U << 3);
	static constexpr auto UDP_TERMINATOR = (1U << 4);
};
}  // namespace ltcetcparams

class LtcEtcParamsStore {
public:
	static void Update(const ltcetcparams::Params *pLtcEtcParams) {
		ConfigStore::Get()->Update(configstore::Store::LTCETC, pLtcEtcParams, sizeof(struct ltcetcparams::Params));
	}

	static void Copy(struct ltcetcparams::Params *pLtcEtcParams) {
		ConfigStore::Get()->Copy(configstore::Store::LTCETC, pLtcEtcParams, sizeof(struct ltcetcparams::Params));
	}
};

class LtcEtcParams {
public:
	LtcEtcParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct ltcetcparams::Params *ptLtcEtcParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    ltcetcparams::Params m_Params;
};

#endif /* LTCETCPARAMS_H_ */
