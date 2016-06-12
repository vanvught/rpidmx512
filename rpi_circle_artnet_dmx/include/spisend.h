/**
 * @file spisend.h
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef SPISEND_H_
#define SPISEND_H_

#include <stdint.h>
#include <circle/interrupt.h>

#include "ws28xxstripe.h"
#include "lightset.h"

class SPISend: public LightSet {
public:
	SPISend(CInterruptSystem *);
	~SPISend(void);

	void Start(void);
	void Stop(void);

	void SetData(const uint8_t, const uint8_t *, const uint16_t);

	void SetLEDType(TWS28XXType);
	const TWS28XXType GetLEDType(void);

	void SetLEDCount(unsigned);
	unsigned GetLEDCount(void);

private:
	CInterruptSystem	*m_pInterrupt;
	CWS28XXStripe		*m_pLEDStripe;
	TWS28XXType			m_LEDType;
	unsigned			m_nLEDCount;
};

#endif /* SPISEND_H_ */
