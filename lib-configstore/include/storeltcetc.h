/**
 * @file storeltcetc.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STORELTCETC_H_
#define STORELTCETC_H_

#include <cassert>

#include "ltcetcparams.h"

#include "configstore.h"

class StoreLtcEtc final: public LtcEtcParamsStore {
public:
	StoreLtcEtc() {
		assert(s_pThis == nullptr);
		s_pThis = this;
	}

	void Update(const ltcetcparams::Params *pLtcEtcParams) override {
		ConfigStore::Get()->Update(configstore::Store::LTCETC, pLtcEtcParams, sizeof(struct ltcetcparams::Params));
	}

	void Copy(struct ltcetcparams::Params *pLtcEtcParams) override {
		ConfigStore::Get()->Copy(configstore::Store::LTCETC, pLtcEtcParams, sizeof(struct ltcetcparams::Params));
	}

	static StoreLtcEtc *Get() {
		return s_pThis;
	}

private:
	static StoreLtcEtc *s_pThis;
};

#endif /* STORELTCETC_H_ */
