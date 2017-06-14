/**
 * @file dmxsender.h
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

#ifndef DMXSENDER_H_
#define DMXSENDER_H_

#include <stdint.h>
#include <stdbool.h>

#include "lightset.h"

class DMXSender: public LightSet {
public:
	DMXSender(void);
	~DMXSender(void);

	void Start(void);
	void Stop(void);

	void SetBreakTime(const uint32_t);
	const uint32_t GetBreakTime(void);

	void SetMabTime(const uint32_t);
	const uint32_t GetMabTime(void);

	void SetPeriodTime(const uint32_t);
	const uint32_t GetPeriodTime(void);

	void SetData(const uint8_t, const uint8_t *, const uint16_t);
private:
	bool m_bIsStarted;
};

#endif /* DMXSENDER_H_ */
