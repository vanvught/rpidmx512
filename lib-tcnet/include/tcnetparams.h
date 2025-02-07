/**
 * @file tcnetparams.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef TCNETPARAMS_H_
#define TCNETPARAMS_H_

#include <cstdint>

#include "tcnet.h"
#include "tcnetpackets.h"
#include "configstore.h"

namespace tcnetparams {

struct Params {
	uint32_t nSetList;
	char aNodeName[TCNET_NODE_NAME_LENGTH];
	uint8_t nLayer;
	uint8_t nTimeCodeType;
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr auto NODE_NAME = (1U << 0);
	static constexpr auto LAYER = (1U << 1);
	static constexpr auto TIMECODE_TYPE = (1U << 2);
	static constexpr auto USE_TIMECODE = (1U << 3);
};
}  // namespace tcnetparams

class TCNetParamsStore {
public:
	static void Update(const struct tcnetparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::TCNET, pParams, sizeof(struct tcnetparams::Params));
	}

	static void Copy(struct tcnetparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::TCNET, pParams, sizeof(struct tcnetparams::Params));
	}
};

class TCNetParams {
public:
	TCNetParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct tcnetparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(TCNet *pTCNet);

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) {
		return (m_Params.nSetList & nMask) == nMask;
	}

private:
    tcnetparams::Params	m_Params;
};

#endif /* TCNETPARAMS_H_ */
