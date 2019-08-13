/**
 * @file motorparams.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MOTORPARAMS_H_
#define MOTORPARAMS_H_

#include <stdint.h>

#include "l6470.h"

struct TMotorParams {
    uint32_t nSetList;
	float fStepAngel;
	float fVoltage;
	float fCurrent;
	float fResistance;
	float fInductance;
}__attribute__((packed));

enum TMotorParamsMask {
	MOTOR_PARAMS_MASK_STEP_ANGEL = (1 << 0),
	MOTOR_PARAMS_MASK_VOLTAGE = (1 << 1),
	MOTOR_PARAMS_MASK_CURRENT = (1 << 2),
	MOTOR_PARAMS_MASK_RESISTANCE = (1 << 3),
	MOTOR_PARAMS_MASK_INDUCTANCE = (1 << 4)
};

class MotorParamsStore {
public:
	virtual ~MotorParamsStore(void);

	virtual void Update(uint8_t nMotorIndex, const struct TMotorParams *ptMotorParams)=0;
	virtual void Copy(uint8_t nMotorIndex, struct TMotorParams *ptMotorParams)=0;
};

class MotorParams {
public:
	MotorParams(MotorParamsStore *pMotorParamsStore=0);
	~MotorParams(void);

	bool Load(uint8_t nMotorIndex);
	void Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	bool Builder(uint8_t nMotorIndex, const struct TMotorParams *ptMotorParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t nMotorIndex, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(L6470 *pL6470);

	void Dump(void);

	float GetStepAngel(void) {
		return m_tMotorParams.fStepAngel;
	}

	float GetVoltage(void) {
		return m_tMotorParams.fVoltage;
	}

	float GetCurrent(void) {
		return m_tMotorParams.fCurrent;
	}

	float GetResistance(void) {
		return m_tMotorParams.fResistance;
	}

	float GetInductance(void) {
		return m_tMotorParams.fInductance;
	}

private:
    void callbackFunction(const char *s);
	bool isMaskSet(uint32_t) const;
	float calcIntersectSpeed(void) const;
	uint32_t calcIntersectSpeedReg(float) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    MotorParamsStore *m_pMotorParamsStore;
    struct TMotorParams m_tMotorParams;
    char m_aFileName[16];
};

#endif /* MOTORPARAMS_H_ */
