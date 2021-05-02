/**
 * @file l6470params.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

struct TL6470Params {
    uint32_t nSetList;
    //
    float fMinSpeed;
    float fMaxSpeed;
    float fAcc;
    float fDec;
    uint8_t nKvalHold;
    uint8_t nKvalRun;
    uint8_t nKvalAcc;
    uint8_t nKvalDec;
    uint8_t nMicroSteps;
} __attribute__((packed));

struct L6470ParamsMask {
	static constexpr auto MIN_SPEED   = (1U << 0);
	static constexpr auto MAX_SPEED   = (1U << 1);
	static constexpr auto ACC         = (1U << 2);
	static constexpr auto DEC         = (1U << 3);
	static constexpr auto KVAL_HOLD   = (1U << 4);
	static constexpr auto KVAL_RUN    = (1U << 5);
	static constexpr auto KVAL_ACC    = (1U << 6);
	static constexpr auto KVAL_DEC    = (1U << 7);
	static constexpr auto MICRO_STEPS = (1U << 8);
};

class L6470ParamsStore {
public:
	virtual ~L6470ParamsStore() {}

	virtual void Update(uint8_t nMotorIndex, const struct TL6470Params *ptL6470Params)=0;
	virtual void Copy(uint8_t nMotorIndex, struct TL6470Params *ptL6470Params)=0;
};

class L6470Params {
public:
	L6470Params(L6470ParamsStore *pL6470ParamsStore=nullptr);

	bool Load(uint8_t nMotorIndex);
	void Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(uint8_t nMotorIndex, const struct TL6470Params *ptL6470Params, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(uint8_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(L6470 *);

	void Dump();

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) {
    	return (m_tL6470Params.nSetList & nMask) == nMask;
    }

private:
	L6470ParamsStore *m_pL6470ParamsStore;
    struct TL6470Params m_tL6470Params;
    char m_aFileName[16];
};

#endif /* L6470PARAMS_H_ */
