/**
 * @file storee131.h
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

#ifndef STOREE131_H_
#define STOREE131_H_

#include "e131params.h"

#include "spiflashstore.h"

class StoreE131 final: public E131ParamsStore {
public:
	StoreE131();

	void Update(const struct TE131Params *pE131Params) override {
		SpiFlashStore::Get()->Update(STORE_E131, pE131Params, sizeof(struct TE131Params));
	}

	void Copy(struct TE131Params *pE131Params) override {
		SpiFlashStore::Get()->Copy(STORE_E131, pE131Params, sizeof(struct TE131Params));
	}

	static StoreE131* Get() {
		return s_pThis;
	}

private:
	static StoreE131 *s_pThis;
};

#endif /* STOREE131_H_ */
