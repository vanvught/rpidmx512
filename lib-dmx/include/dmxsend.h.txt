/**
 * @file dmxsend.h
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

#ifndef DMXSEND_H_
#define DMXSEND_H_

#include <stdint.h>
#include <stdbool.h>

#include "dmx.h"
#include "lightset.h"

enum TDMXSendState {
	DMXSendIdle,
	DMXSendBreak,
	DMXSendMAB,
	DMXSendData,
	DMXSendInterPacket,
	DMXSendUnknown
};


class DMXSend: public LightSet {
public:
	DMXSend(void);
	~DMXSend(void);

	void Start(void);
	void Stop(void);

	void SetDataLength(const uint16_t);
	uint16_t GetDataLength(void);

	void SetBreakTime(const uint32_t);
	uint32_t GetBreakTime(void);
	void SetMabTime(const uint32_t);
	uint32_t GetMabTime(void);
	void SetPeriodTime(const uint32_t);
	uint32_t GetPeriodTime(void);
	uint32_t GetPeriodTimeRequested(void);

	void SetData(const uint8_t, const uint8_t *, const uint16_t);

private:
	void SerialIRQHandler (void);
	void TimerIRQHandler (void);

	void ClearOutputData(void);
	bool SetDataTry(const uint8_t *, const uint16_t);	// returns FALSE if transfer is active
};

#endif /* DMXSEND_H_ */
