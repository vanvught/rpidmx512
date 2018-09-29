/**
 * @file dmxmulti.h
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
#ifndef DMXMULTI_H_
#define DMXMULTI_H_

#include "dmx_multi.h"

#include "dmx.h"

class DmxMulti: public DmxSet {
public:
	DmxMulti(void);
	~DmxMulti(void);

	void SetPortDirection(uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData = false);

	inline void SetDmxBreakTime(uint32_t nBreakTime) {
		dmx_multi_set_output_break_time(nBreakTime);
	}

	inline  uint32_t GetDmxBreakTime(void) {
		return dmx_multi_get_output_break_time();
	}

	inline void SetDmxMabTime(uint32_t nMabTime) {
		dmx_multi_set_output_mab_time(nMabTime);
	}

	inline uint32_t GetDmxMabTime(void) {
		return dmx_multi_get_output_mab_time();
	}

	inline uint32_t GetDmxPeriodTime(void) {
		return dmx_multi_get_output_period();
	}

	void RdmSendRaw(uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength);

	const uint8_t *RdmReceive(uint8_t nPort);
	const uint8_t *RdmReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut);

private:
	bool m_IsInitDone;
};

#endif /* DMXMULTI_H_ */
