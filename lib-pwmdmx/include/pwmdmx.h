/**
 * @file pwmdmx.h
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

#ifndef PWMDMX_H_
#define PWMDMX_H_

#include "lightset.h"

enum TPwmDmxMode {
	PWMDMX_MODE_PWMLED,
	PWMDMX_MODE_SERVO
};

enum TPwmDmxChip {
	PWMDMX_CHIP_TLC59711,
	PWMDMX_CHIP_PCA9685
};

class PwmDmx {
public:
	PwmDmx(void);
	~PwmDmx(void);

	void Dump(void);

	TPwmDmxMode GetPwmDmxMode(bool &IsSet) const;
	TPwmDmxChip GetPwmDmxChip(bool &IsSet) const;

	LightSet *GetLightSet(void) const;

private:
	bool isMaskSet(uint16_t nMask) const;

public:
	static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);

private:
    uint16_t m_bSetList;
	TPwmDmxMode m_tPwmDmxMode;
	TPwmDmxChip m_tPwmDmxChip;
	LightSet *m_pLightSet;
};

#endif /* PWMDMX_H_ */
