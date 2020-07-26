/**
 * @file storeremoteconfig.h
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

#ifndef STOREREMOTECONFIG_H_
#define STOREREMOTECONFIG_H_

#include "remoteconfigparams.h"

#include "spiflashstore.h"

class StoreRemoteConfig final: public RemoteConfigParamsStore {
public:
	StoreRemoteConfig();

	void Update(const struct TRemoteConfigParams *pRemoteConfigParams) override {
		SpiFlashStore::Get()->Update(STORE_RCONFIG, pRemoteConfigParams, sizeof(struct TRemoteConfigParams));
	}

	void Copy(struct TRemoteConfigParams *pRemoteConfigParams) override {
		SpiFlashStore::Get()->Copy(STORE_RCONFIG, pRemoteConfigParams, sizeof(struct TRemoteConfigParams));
	}

	static StoreRemoteConfig *Get() {
		return s_pThis;
	}

private:
	static StoreRemoteConfig *s_pThis;
};

#endif /* STOREREMOTECONFIG_H_ */
