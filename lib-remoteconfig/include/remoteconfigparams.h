/**
 * @file remoteconfigparams.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "remoteconfig.h"

struct TRemoteConfigParams {
    uint32_t nSetList;
	bool bDisable;
	bool bDisableWrite;
	bool bEnableReboot;
	bool bEnableUptime;
	char aDisplayName[REMOTE_CONFIG_DISPLAY_NAME_LENGTH];
	bool bDisableRdmNetLlrpOnly;
} __attribute__((packed));

struct RemoteConfigParamsMask {
	static constexpr auto DISABLE = (1U << 0);
	static constexpr auto DISABLE_WRITE = (1U << 1);
	static constexpr auto ENABLE_REBOOT = (1U << 2);
	static constexpr auto ENABLE_UPTIME = (1U << 3);
	static constexpr auto DISPLAY_NAME = (1U << 4);
	static constexpr auto DISABLE_RDMNET_LLRP_ONLY = (1U << 5);
};

class RemoteConfigParamsStore {
public:
	virtual ~RemoteConfigParamsStore() {
	}

	virtual void Update(const struct TRemoteConfigParams *pRemoteConfigParams)=0;
	virtual void Copy(struct TRemoteConfigParams *pRemoteConfigParams)=0;
};

class RemoteConfigParams {
public:
	RemoteConfigParams(RemoteConfigParamsStore *pRemoteConfigParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TRemoteConfigParams *pRemoteConfigParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(RemoteConfig *);

	void Dump();

	bool IsDisableRdmNetLlrpOnly() const {
		return m_tRemoteConfigParams.bDisableRdmNetLlrpOnly;
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    void SetBool(const uint8_t nValue, bool& nProperty, const uint32_t nMask);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tRemoteConfigParams.nSetList & nMask) == nMask;
    }

private:
	RemoteConfigParamsStore *m_pRemoteConfigParamsStore;
    TRemoteConfigParams m_tRemoteConfigParams;
};

#endif /* REMOTECONFIGPARAMS_H_ */
