/**
 * @file systimereader.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_SYSTIMEREADER_H_
#define H3_SYSTIMEREADER_H_

#include <stdint.h>
#include <time.h>

#include "ltc.h"
#include "midi.h"

class SystimeReader {
public:
	SystimeReader (struct TLtcDisabledOutputs *pLtcDisabledOutputs, uint8_t nFps);

	void Start(bool bAutoStart = false);
	void Run();

	void Print();

	//
	void HandleRequest(void *pBuffer = nullptr, uint32_t nBufferLength = 0);

	// Control
	void ActionStart();
	void ActionStop();
	void ActionSetRate(const char *pTimeCodeRate);

	static SystimeReader *Get() {
		return s_pThis;
	}

private:
	void HandleUdpRequest();

private:
	TLtcDisabledOutputs *m_ptLtcDisabledOutputs;
	uint8_t m_nFps;
	uint32_t m_nTimer0Interval;
	time_t m_ntimePrevious{0};
	struct _midi_send_tc m_tMidiTimeCode;
	int32_t m_nHandle{-1};
	char m_Buffer[64];
	uint16_t m_nBytesReceived{0};
	bool m_bIsStarted{false};

	static SystimeReader *s_pThis;
};

#endif /* H3_SYSTIMEREADER_H_ */
