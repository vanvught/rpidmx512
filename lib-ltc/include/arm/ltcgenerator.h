/**
 * @file ltcgenerator.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_LTCGENERATOR_H_
#define ARM_LTCGENERATOR_H_

#include <cstdint>

#include "ltc.h"

#include "hardware.h"

namespace ltcgenerator {
enum class Direction {
	DIRECTION_FORWARD, DIRECTION_BACKWARD
};

enum class Pitch {
	PITCH_NORMAL, PITCH_FASTER, PITCH_SLOWER
};
}  // namespace ltcgenerator

class LtcGenerator {
public:
	LtcGenerator(const struct ltc::TimeCode *pStartLtcTimeCode, const struct ltc::TimeCode *pStopLtcTimeCode, bool bSkipFree = false, bool bIgnoreStart = false, bool bIgnoreStop = false);

	~LtcGenerator() {
		Stop();
	}

	void Start();
	void Stop();

	void Run() {
		Update();
		HandleButtons();

		if (m_State == STARTED) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
		} else {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
		}
	}

	void Print();

	void HandleRequest(char *pBuffer = nullptr, uint32_t nBufferLength = 0);

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

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	static LtcGenerator *Get() {
		return s_pThis;
	}

private:
	void HandleButtons();
	void Update();
	void Increment();
	void Decrement();
	bool PitchControl();
	void SetPitch(const char *pTimeCodePitch, uint32_t nSize);
	void SetSkip(const char *pSeconds, uint32_t nSize, const ltcgenerator::Direction direction);
	void SetTimeCode(int32_t nSeconds);

	int32_t GetSeconds(const struct ltc::TimeCode& timecode) {
		int32_t nSeconds = timecode.nHours;
		nSeconds *= 60U;
		nSeconds += timecode.nMinutes;
		nSeconds *= 60U;
		nSeconds += timecode.nSeconds;

		return nSeconds;
	}

	void static StaticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	ltc::TimeCode *m_pStartLtcTimeCode;
	ltc::TimeCode *m_pStopLtcTimeCode;
	bool m_bSkipFree;
	bool m_bIgnoreStart;
	bool m_bIgnoreStop;
	bool m_bDropFrame;
	int32_t m_nStartSeconds;
	int32_t m_nStopSeconds;

	uint32_t m_nPitchTicker { 1 };
	uint32_t m_nPitchPrevious { 0 };
	float m_fPitchControl { 0 };
	uint32_t m_nButtons { 0 };
	int32_t m_nHandle { -1 };
	uint32_t m_nBytesReceived { 0 };

	char *m_pUdpBuffer { nullptr };

	uint8_t m_nFps { 0 };

	ltcgenerator::Direction m_tDirection { ltcgenerator::Direction::DIRECTION_FORWARD };
	ltcgenerator::Pitch m_tPitch { ltcgenerator::Pitch::PITCH_FASTER };

	enum {
		STOPPED, STARTED, LIMIT
	} m_State { STOPPED };

	static inline LtcGenerator *s_pThis;
};

#endif /* ARM_LTCGENERATOR_H_ */
