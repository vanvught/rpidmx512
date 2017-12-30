/**
 * @file dmxrdm.cpp
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

#ifndef DMXRDM_H_
#define DMXRDM_H_

#include <stdint.h>

#include "dmx.h"
#include "gpio.h"

enum TDmxRdmPortDirection {
	DMXRDM_PORT_DIRECTION_OUTP = 1,
	DMXRDM_PORT_DIRECTION_INP = 2
};

struct TDmxStatistics {
	uint32_t MarkAfterBreak;
	uint32_t SlotsInPacket;
	uint32_t BreakToBreak;
	uint32_t SlotToSlot;
};

struct TDmxData {
	uint8_t Data[DMX_DATA_BUFFER_SIZE];
	struct TDmxStatistics Statistics;
};

class DmxRdm {
public:
	DmxRdm(uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION);
	~DmxRdm(void);

	void SetPortDirection(TDmxRdmPortDirection, bool bEnableData = false);

public: // DMX
	uint32_t GetUpdatesPerSecond(void) const;
	const uint8_t *GetDmxCurrentData(void);
	const uint8_t *GetDmxAvailable(void);

	uint32_t GetDmxBreakTime(void) const;
	void SetDmxBreakTime(uint32_t);

	uint32_t GetDmxMabTime(void) const;
	void SetDmxMabTime(uint32_t);

	uint32_t GetDmxPeriodTime(void) const;
	void SetDmxPeriodTime(uint32_t);

public: // RDM

private:
};


#endif /* DMXRDM_H_ */
