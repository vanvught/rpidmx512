/**
 * @file storepca9685.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STOREPCA9685_H_
#define STOREPCA9685_H_

#include <cstdint>

#include "pca9685dmxparams.h"
#include "pca9685dmxstore.h"

#include "configstore.h"

class StorePCA9685 final: public PCA9685DmxParamsStore, public PCA9685DmxStore {
public:
	StorePCA9685();

	void Update(const struct pca9685dmxparams::Params *pParams) override {
		ConfigStore::Get()->Update(configstore::Store::PCA9685, pParams, sizeof(struct pca9685dmxparams::Params));
	}

	void Copy(struct pca9685dmxparams::Params *pParams) override {
		ConfigStore::Get()->Copy(configstore::Store::PCA9685, pParams, sizeof(struct pca9685dmxparams::Params));
	}

	void SaveDmxStartAddress(uint16_t nDmxStartAddress) override {
		ConfigStore::Get()->Update(configstore::Store::PCA9685, __builtin_offsetof(struct pca9685dmxparams::Params, nDmxStartAddress), &nDmxStartAddress, sizeof(uint32_t), pca9685dmxparams::Mask::DMX_START_ADDRESS);
	}

	static StorePCA9685 *Get() {
		return s_pThis;
	}

private:
	static StorePCA9685 *s_pThis;
};

#endif /* STOREPCA9685_H_ */
