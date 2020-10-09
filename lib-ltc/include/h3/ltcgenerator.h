/**
 * @file ltcgenerator.h
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

#ifndef H3_LTCGENERATOR_H_
#define H3_LTCGENERATOR_H_

#include <stdint.h>

#include "ltc.h"

enum TLtcGeneratorDirection {
	LTC_GENERATOR_FORWARD,
	LTC_GENERATOR_BACKWARD
};

enum TLtcGeneratorPitch {
	LTC_GENERATOR_NORMAL,
	LTC_GENERATOR_FASTER,
	LTC_GENERATOR_SLOWER
};

class LtcGenerator {
public:
	LtcGenerator(const struct TLtcTimeCode *pStartLtcTimeCode, const struct TLtcTimeCode *pStopLtcTimeCode, struct TLtcDisabledOutputs *pLtcDisabledOutputs, bool bSkipFree = false);
	~LtcGenerator();

	void Start();
	void Stop();

	void Run();

	void Print();

	//
	void HandleRequest(void *pBuffer = nullptr, uint32_t nBufferLength = 0);

	// Control
	void ActionStart(bool bDoReset = true);
	void ActionStop();
	void ActionResume();
	void ActionReset();
	void ActionSetStart(const char *pTimeCode);
	void ActionSetStop(const char *pTimeCode);
	void ActionSetRate(const char *pTimeCodeRate);
	void ActionGoto(const char *pTimeCode);
	void ActionSetDirection(const char *pTimeCodeDirection);
	void ActionSetPitch(float fTimeCodePitch);
	void ActionForward(int32_t nSeconds);
	void ActionBackward(int32_t nSeconds);

	static LtcGenerator* Get() {
		return s_pThis;
	}

private:
	void HandleButtons();
	void HandleUdpRequest();
	void Update();
	void Increment();
	void Decrement();
	bool PitchControl();
	void SetPitch(const char *pTimeCodePitch, uint32_t nSize);
	void SetSkip(const char *pSeconds, uint32_t nSize, TLtcGeneratorDirection tDirection);
	void SetTimeCode(int32_t nSeconds);
	int32_t GetSeconds(const TLtcTimeCode &timecode);

private:
	struct TLtcTimeCode *m_pStartLtcTimeCode;
	int32_t m_nStartSeconds;
	struct TLtcTimeCode *m_pStopLtcTimeCode;
	int32_t m_nStopSeconds;
	bool m_bSkipFree;
	uint8_t m_nFps{0};
	TLtcGeneratorDirection m_tDirection{LTC_GENERATOR_FORWARD};
	float m_fPitchControl{0};
	uint32_t m_nPitchTicker{1};
	uint32_t m_nPitchPrevious{0};
	TLtcGeneratorPitch m_tPitch{LTC_GENERATOR_FASTER};
	uint32_t m_nTimer0Interval{0};
	uint32_t m_nButtons{0};
	int m_nHandle{-1};
	char m_Buffer[64];
	uint16_t m_nBytesReceived{0};
	enum {
		STOPPED,
		STARTED,
		LIMIT
	} m_State{STOPPED};

	static LtcGenerator *s_pThis;
};

#endif /* H3_LTCGENERATOR_H_ */
