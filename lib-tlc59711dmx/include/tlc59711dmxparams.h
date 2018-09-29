/**
 * @file tlc59711dmxparams.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef PWMDMXTLC59711PARAMS_H_
#define PWMDMXTLC59711PARAMS_H_

#include <stdint.h>

#include "tlc59711dmx.h"

class TLC59711DmxParams {
public:
	TLC59711DmxParams(void);
	~TLC59711DmxParams(void);

	bool Load(void);
	void Set(TLC59711Dmx *);
	void Dump(void);

	bool IsSetLedType(void) const;
	bool IsSetLedCount(void) const;

public:
	static const char *GetLedTypeString(TTLC59711Type tTTLC59711Type);

private:
	bool isMaskSet(uint32_t nMask) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);

private:
    uint32_t m_bSetList;
	TTLC59711Type m_LEDType;
	uint8_t m_nLEDCount;
	uint16_t m_nDmxStartAddress;
    uint32_t m_nSpiSpeedHz;
};

#endif /* PWMDMXTLC59711PARAMS_H_ */
