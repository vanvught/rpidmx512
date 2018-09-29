/**
 * @file l6470params.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef L6470PARAMS_H_
#define L6470PARAMS_H_

#include <stdint.h>

#include "l6470.h"

class L6470Params {
public:
	L6470Params(const char *);
	~L6470Params(void);

	void Set(L6470 *);
	void Dump(void);

private:
	bool isMaskSet(uint32_t) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    uint32_t m_bSetList;
    float m_fMinSpeed;
    float m_fMaxSpeed;
    float m_fAcc;
    float m_fDec;
    uint8_t m_nKvalHold;
    uint8_t m_nKvalRun;
    uint8_t m_nKvalAcc;
    uint8_t m_nKvalDec;
    uint8_t m_nMicroSteps;
};

#endif /* L6470PARAMS_H_ */
