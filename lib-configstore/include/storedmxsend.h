/**
 * @file storedmxsend.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STOREDMXSEND_H_
#define STOREDMXSEND_H_

#include "dmxparams.h"

#include "configstore.h"

class StoreDmxSend final: public DmxParamsStore {
public:
	StoreDmxSend();

	void Update(const struct TDmxParams *pDmxParams) override {
		ConfigStore::Get()->Update(configstore::Store::DMXSEND, pDmxParams, sizeof(struct TDmxParams));
	}

	void Copy(struct TDmxParams *pDmxParams) override {
		ConfigStore::Get()->Copy(configstore::Store::DMXSEND, pDmxParams, sizeof(struct TDmxParams));
	}

	static StoreDmxSend *Get() {
		return s_pThis;
	}

private:
	static StoreDmxSend *s_pThis;
};

#endif /* STOREDMXSEND_H_ */
