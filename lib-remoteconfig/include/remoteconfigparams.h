/**
 * @file remoteconfigparams.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
	bool bDisabled;
	bool bDisableWrite;
	bool bEnableReboot;
	bool bEnableUptime;
	uint8_t aDisplayName[REMOTE_CONFIG_DISPLAY_NAME_LENGTH];
};

enum TRemoteConfigParamsMask {
	REMOTE_CONFIG_PARAMS_DISABLED = (1 << 0),
	REMOTE_CONFIG_PARAMS_DISABLE_WRITE = (1 << 1),
	REMOTE_CONFIG_PARAMS_ENABLE_REBOOT = (1 << 2),
	REMOTE_CONFIG_PARAMS_ENABLE_UPTIME = (1 << 3),
	REMOTE_CONFIG_PARAMS_DISPLAY_NAME = (1 << 4)
};

class RemoteConfigParamsStore {
public:
	virtual ~RemoteConfigParamsStore(void);

	virtual void Update(const struct TRemoteConfigParams *pTRemoteConfigParams)=0;
	virtual void Copy(struct TRemoteConfigParams *pTRemoteConfigParams)=0;
};

class RemoteConfigParams {
public:
	RemoteConfigParams(RemoteConfigParamsStore *pTRemoteConfigParamsStore = 0);
	~RemoteConfigParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TRemoteConfigParams *pRemoteConfigParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(RemoteConfig *);

	void Dump(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;

private:
	RemoteConfigParamsStore *m_pRemoteConfigParamsStore;
    struct TRemoteConfigParams m_tRemoteConfigParams;
};

#endif /* REMOTECONFIGPARAMS_H_ */
