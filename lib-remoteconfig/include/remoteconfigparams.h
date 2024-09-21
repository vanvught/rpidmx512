/**
 * @file remoteconfigparams.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef REMOTECONFIGPARAMS_H_
#define REMOTECONFIGPARAMS_H_

#include <cstdint>

#include "remoteconfig.h"
#include "configstore.h"

namespace remoteconfigparams {
struct Params {
    uint32_t nSetList;
	uint8_t NotUsed0;
	uint8_t NotUsed1;
	uint8_t NotUsed2;
	uint8_t NotUsed3;
	char aDisplayName[remoteconfig::DISPLAY_NAME_LENGTH];
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 48, "struct Params is too large");

struct Mask {
	static constexpr uint32_t DISABLE = (1U << 0);
	static constexpr uint32_t DISABLE_WRITE = (1U << 1);
	static constexpr uint32_t ENABLE_REBOOT = (1U << 2);
	static constexpr uint32_t ENABLE_UPTIME = (1U << 3);
	static constexpr uint32_t DISPLAY_NAME = (1U << 4);
	static constexpr uint32_t ENABLE_FACTORY = (1U << 5);
};
}  // namespace remoteconfigparams

class RemoteConfigParamsStore {
public:
	static void Update(const struct remoteconfigparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::RCONFIG, pParams, sizeof(struct remoteconfigparams::Params));
	}

	static void Copy(struct remoteconfigparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::RCONFIG, pParams, sizeof(struct remoteconfigparams::Params));
	}
};

class RemoteConfigParams {
public:
	RemoteConfigParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct remoteconfigparams::Params *pRemoteConfigParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(RemoteConfig *);

	const char *GetDisplayName() const {
		return m_Params.aDisplayName;
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
	void callbackFunction(const char *pLine);
	void SetBool(const uint8_t nValue, const uint32_t nMask);
	bool isMaskSet(uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}

private:
	remoteconfigparams::Params m_Params;
};

#endif /* REMOTECONFIGPARAMS_H_ */
