/**
 * @file mcpbuttons.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

enum class RunStatus {
	IDLE, CONTINUE, REBOOT, TC_RESET
};

class McpButtons {
public:
	McpButtons(ltc::Source tLtcReaderSource, bool bUseAltFunction, int32_t nSkipSeconds, bool bRotaryHalfStep);

	bool Check();
	bool Wait(ltc::Source& tLtcReaderSource, struct ltc::TimeCode& StartTimeCode, struct ltc::TimeCode& StopTimeCode);

	bool IsConnected() const {
		return m_bIsConnected;
	}

	void SetRunGpsTimeClient(bool bRunGpsTimeClient) {
		m_bRunGpsTimeClient = bRunGpsTimeClient;
	}

	void Run();

private:
	uint32_t LedBlink(uint8_t nPortB);
	void HandleActionLeft(ltc::Source& ltcSource);
	void HandleActionRight(ltc::Source& ltcSource);
	void HandleActionSelect(const ltc::Source& ltcSource);
	void HandleRotary(uint8_t nInputAB, ltc::Source& ltcSource);
	void UpdateDisplays(const ltc::Source ltcSource);
	// Running mode
	void HandleRunActionSelect();
	void SetRunState(RunStatus tRunState);
	// Internal
	void HandleInternalTimeCodeStart(struct ltc::TimeCode& timecode);
	void HandleInternalTimeCodeStop(struct ltc::TimeCode& timecode);
	void HandleInternalTimeCodeFps(struct ltc::TimeCode& timecode);
	void HandleInternalKeyEsc();

private:
	HAL_I2C m_I2C;
	DisplayEditTimeCode displayEditTimeCode;
	DisplayEditFps displayEditFps;
	enum {
		SOURCE_SELECT, EDIT_TIMECODE_START, EDIT_TIMECODE_STOP, EDIT_FPS
	} m_State { SOURCE_SELECT };
	ltc::Source m_tLtcReaderSource;
	bool m_bUseAltFunction;
	int32_t m_nSkipSeconds;
	bool m_bIsConnected { false };
	uint8_t m_nPortAPrevious { 0 };
	uint8_t m_nPortB { 0 };
	uint32_t m_nMillisPrevious { 0 };
	uint32_t m_nLedTicker { 0 };
	uint32_t m_nLedTickerMax { 20 };	// 10 seconds
	RotaryEncoder m_RotaryEncoder;
	uint8_t m_tRotaryDirection { RotaryEncoder::NONE };
	RunStatus m_tRunStatus { RunStatus::IDLE };
	uint32_t m_nSelectMillis { 0 };
	int m_nKey { input::KEY_NOT_DEFINED };
	char m_aTimeCode[ltc::timecode::CODE_MAX_LENGTH];
	bool m_bRunGpsTimeClient { false };
};

#endif /* SOURCESELECT_H_ */
