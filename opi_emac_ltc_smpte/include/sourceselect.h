/**
 * @file sourceselect.h
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

#ifndef SOURCESELECT_H_
#define SOURCESELECT_H_

#include <stdint.h>

#include "ltcparams.h"

#include "rotaryencoder.h"

#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "input.h"

#include "hal_i2c.h"

enum TRunStatus {
	RUN_STATUS_IDLE,
	RUN_STATUS_CONTINUE,
	RUN_STATUS_REBOOT
};

class SourceSelect {
public:
	SourceSelect(TLtcReaderSource tLtcReaderSource, TLtcDisabledOutputs *ptLtcDisabledOutputs);

	bool Check(void);
	bool Wait(TLtcReaderSource& tLtcReaderSource, TLtcTimeCode& StartTimeCode, TLtcTimeCode& StopTimeCode);

	bool IsConnected(void) {
		return m_bIsConnected;
	}

	void Run(void);

private:
	void LedBlink(uint8_t nPortB);
	void HandleActionLeft(TLtcReaderSource& tLtcReaderSource);
	void HandleActionRight(TLtcReaderSource& tLtcReaderSource);
	void HandleActionSelect(const TLtcReaderSource& tLtcReaderSource);
	void HandleRotary(uint8_t nInputAB, TLtcReaderSource& tLtcReaderSource);
	void UpdateDisplays(const TLtcReaderSource& tLtcReaderSource);
	// Running mode
	void HandleRunActionSelect(void);
	void SetRunState(TRunStatus tRunState);
	// Internal
	void HandleInternalTimeCodeStart(TLtcTimeCode &timecode);
	void HandleInternalTimeCodeStop(TLtcTimeCode &timecode);
	void HandleInternalTimeCodeFps(TLtcTimeCode &timecode);
	void HandleInternalKeyEsc(void);

private:
	HAL_I2C m_I2C;
	DisplayEditTimeCode displayEditTimeCode;
	DisplayEditFps displayEditFps;
	enum {
		SOURCE_SELECT,
		EDIT_TIMECODE_START,
		EDIT_TIMECODE_STOP,
		EDIT_FPS
	} m_State = SOURCE_SELECT;
	TLtcReaderSource m_tLtcReaderSource;
	struct TLtcDisabledOutputs *m_ptLtcDisabledOutputs;
	bool m_bIsConnected;
	uint8_t m_nPortAPrevious;
	uint8_t m_nPortB;
	uint32_t m_nMillisPrevious;
	RotaryEncoder m_RotaryEncoder;
	uint8_t m_tRotaryDirection;
	TRunStatus m_tRunStatus;
	uint32_t m_nSelectMillis;
	int m_nKey = INPUT_KEY_NOT_DEFINED;
	char m_aTimeCode[TC_CODE_MAX_LENGTH];
};

#endif /* SOURCESELECT_H_ */
