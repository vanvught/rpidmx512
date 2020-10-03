/**
 * @file storeoscclient.h
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

#ifndef STOREOSCCLIENT_H_
#define STOREOSCCLIENT_H_

#include "oscclientparams.h"

#include "spiflashstore.h"

class StoreOscClient final: public OscClientParamsStore {
public:
	StoreOscClient();

	void Update(const struct TOscClientParams *pOscClientParams) override {
		SpiFlashStore::Get()->Update(STORE_OSC_CLIENT, pOscClientParams, sizeof(struct TOscClientParams));
	}

	void Copy(struct TOscClientParams *pOscClientParams) override {
		SpiFlashStore::Get()->Copy(STORE_OSC_CLIENT, pOscClientParams, sizeof(struct TOscClientParams));
	}

	static StoreOscClient *Get() {
		return s_pThis;
	}

private:
	static StoreOscClient *s_pThis;
};

#endif /* STOREOSCCLIENT_H_ */
