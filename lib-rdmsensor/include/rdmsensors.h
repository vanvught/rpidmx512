/**
 * @file rdmsensors.h
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

#ifndef RDMSENSORS_H_
#define RDMSENSORS_H_

#include <stdint.h>

#include "rdmsensor.h"

class RDMSensors {
public:
	RDMSensors(void);
	~RDMSensors(void);

	void Init(void);
	bool Add(RDMSensor *pRDMSensor);
	uint8_t GetCount(void) const;

	const struct TRDMSensorDefintion* GetDefintion(uint8_t nSensor);
	const struct TRDMSensorValues* GetValues(uint8_t nSensor);
	void SetSensorValues(uint8_t nSensor);
	void SetSensorRecord(uint8_t nSensor);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
	RDMSensor **m_pRDMSensor;
	uint8_t m_nCount;
};

#endif /* RDMSENSORS_H_ */
