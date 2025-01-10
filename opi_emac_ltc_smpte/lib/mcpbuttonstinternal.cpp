/**
 * @file mcpbuttonsinternal.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "mcpbuttons.h"

#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "input.h"

#include "display.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayrgb.h"

void McpButtons::HandleInternalTimeCodeStart(struct ltc::TimeCode& StartTimeCode) {
	displayEditTimeCode.HandleKey(m_nKey, StartTimeCode, m_aTimeCode);

//	if (!ltc::g_DisabledOutputs.bMax7219) {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219)) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	} else if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
		LtcDisplayRgb::Get()->Show(m_aTimeCode);
	}

	HandleInternalKeyEsc();
}

void McpButtons::HandleInternalTimeCodeStop(struct ltc::TimeCode& StartTimeCode) {
	displayEditTimeCode.HandleKey(m_nKey, StartTimeCode, m_aTimeCode);

//	if (!ltc::g_DisabledOutputs.bMax7219) {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219)) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	} else if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL)) {
		LtcDisplayRgb::Get()->Show(m_aTimeCode);
	}

	HandleInternalKeyEsc();
}

void McpButtons::HandleInternalTimeCodeFps(struct ltc::TimeCode& StartTimeCode) {
	displayEditFps.HandleKey(m_nKey, StartTimeCode.nType);

	HandleInternalKeyEsc();
}

void McpButtons::HandleInternalKeyEsc() {
	if (m_nKey == input::KEY_ESC) {
		Display::Get()->SetCursor(display::cursor::OFF);
		Display::Get()->SetCursorPos(0, 0);
		Display::Get()->ClearLine(1);
		Display::Get()->ClearLine(2);
		m_State = SOURCE_SELECT;
	}
}
