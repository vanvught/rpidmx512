/**
 * @file slushdmx.h
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

#ifndef SLUSHDMX_H_
#define SLUSHDMX_H_

#include <slushboard.h>
#include <slushmotor.h>
#include <stdint.h>

#include "lightset.h"

#include "l6470dmxmodes.h"

#define SLUSH_DMX_MAX_MOTORS	4

class SlushDmx: public LightSet {
public:
	SlushDmx(bool bUseSPI = true);
	~SlushDmx(void);

	void Start(void);
	void Stop(void);

	void SetData(uint8_t, const uint8_t *, uint16_t);

	bool GetUseSpiBusy(void) const;
	void SetUseSpiBusy(bool);

public:
	void ReadConfigFiles(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
	void UpdateIOPorts(const uint8_t *, const uint16_t);
	bool m_bSetPortA;
	bool m_bSetPortB;
	uint8_t m_nDataPortA;
	uint8_t m_nDataPortB;

private:
	bool m_bUseSpiBusy;

private:
	SlushBoard *m_pBoard;
	SlushMotor	*m_pSlushMotor[SLUSH_DMX_MAX_MOTORS];
	MotorParams *m_pMotorParams[SLUSH_DMX_MAX_MOTORS];
	L6470DmxModes *m_pL6470DmxModes[SLUSH_DMX_MAX_MOTORS];

	uint16_t m_nDmxStartAddressPortA;
	uint16_t m_nDmxStartAddressPortB;

	uint8_t m_nDmxMode;
	uint16_t m_nDmxStartAddress;
};

#endif /* SLUSHDMX_H_ */
