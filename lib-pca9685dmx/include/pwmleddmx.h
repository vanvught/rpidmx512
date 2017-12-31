/**
 * @file pwmleddmx.h
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

#ifndef PWMLEDDMX_H_
#define PWMLEDDMX_H_

#include "lightset.h"

#include "pwmled.h"

class PWMLedDMX: public LightSet {
public:
	PWMLedDMX(void);
	~PWMLedDMX(void);

	void Start(void);
	void Stop(void);

	void SetData(uint8_t, const uint8_t *, uint16_t);

public:
	uint16_t GetDmxStartAddress(void) const;
	void SetDmxStartAddress(uint16_t);

	uint8_t GetI2cAddress(void) const;
	void SetI2cAddress(uint8_t);

	uint8_t GetBoardInstances(void) const;
	void SetBoardInstances(uint8_t);

	uint16_t GetPwmfrequency(void) const;
	void SetPwmfrequency(uint16_t);

	bool GetInvert(void) const;
	void SetInvert(bool);

	bool GetOutDriver(void) const;
	void SetOutDriver(bool);

private:
	void Initialize(void);

private:
	uint16_t m_nDmxStartAddress;
	uint8_t m_nI2cAddress;
	uint8_t m_nBoardInstances;
	uint16_t m_nPwmFrequency;
	bool m_bOutputInvert;
	bool m_bOutputDriver;
	bool m_bIsStarted;
	PWMLed **m_pPWMLed;
	uint8_t *m_pDmxData;
};

#endif /* PWMLEDDMX_H_ */
