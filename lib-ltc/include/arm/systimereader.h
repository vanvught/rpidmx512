/**
 * @file systimereader.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_SYSTIMEREADER_H_
#define ARM_SYSTIMEREADER_H_

#include <cstdint>
#include <time.h>

#include "ltc.h"
#include "midi.h"

class SystimeReader {
public:
	SystimeReader(uint8_t nFps, int32_t nUtcOffset);

	void Start(bool bAutoStart = false);
	void Run();

	void HandleRequest(char *pBuffer = nullptr, uint16_t nBufferLength = 0);

	void ActionStart();
	void ActionStop();
	void ActionSetRate(const char *pTimeCodeRate);

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	static SystimeReader *Get() {
		return s_pThis;
	}

private:
	void SetFps(uint8_t nFps);
	void static StaticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	uint8_t m_nFps;
	int32_t m_nUtcOffset { 0 };
	int32_t m_nHandle { -1 };
	uint32_t m_nBytesReceived { 0 };
	char *m_pUdpBuffer { nullptr };
	time_t m_nTimePrevious { 0 };
	bool m_bIsStarted { false };

	static inline SystimeReader *s_pThis;
};

#endif /* ARM_SYSTIMEREADER_H_ */
