/**
 * @file pca9685leddmxparams.h
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

#ifndef PCA9685DMXLEDPARAMS_H_
#define PCA9685DMXLEDPARAMS_H_

#include <stdint.h>

#include "pca9685dmxparams.h"
#include "pca9685dmxled.h"

class PCA9685DmxLedParams: public PCA9685DmxParams {
public:
	PCA9685DmxLedParams();
	~PCA9685DmxLedParams();

	bool Load();
	void Set(PCA9685DmxLed *);
	void Dump();

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_bSetList & nMask) == nMask;
    }

private:
    uint32_t m_bSetList{0};
    uint8_t m_nI2cAddress{PCA9685_I2C_ADDRESS_DEFAULT};
    uint16_t m_nPwmFrequency{PWMLED_DEFAULT_FREQUENCY};
	bool m_bOutputInvert{false};
	bool m_bOutputDriver{true};
};

#endif /* PCA9685DMXLEDPARAMS_H_ */
