/**
 * @file dmxreceiver.h
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef DMXCONTROLLER_H_
#define DMXCONTROLLER_H_

#include <stdint.h>

#include "dmx.h"

#include "lightset.h"

#include "gpio.h"

class DMXReceiver: public Dmx {
public:
	DMXReceiver(uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION);
	~DMXReceiver(void);

	void SetOutput(LightSet *);

	void Start(void);
	void Stop(void);

	const uint8_t* Run(int16_t &nLength);

private:
	bool IsDmxDataChanged(const uint8_t *, uint16_t);

private:
	LightSet *m_pLightSet;
	bool m_IsActive;
	alignas(uint32_t) uint8_t m_Data[DMX_DATA_BUFFER_SIZE]; // With DMX Start Code
	uint16_t m_nLength;
};

#endif /* DMXCONTROLLER_H_ */
