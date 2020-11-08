/**
 * @file mcpbuttonsinternal.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <ltcdisplayrgb.h>
#include <stdint.h>

#include "mcpbuttons.h"

#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "input.h"

#include "display.h"

#include "ltcdisplaymax7219.h"

void McpButtons::HandleInternalTimeCodeStart(TLtcTimeCode &StartTimeCode) {
	displayEditTimeCode.HandleKey(m_nKey, StartTimeCode, m_aTimeCode);

	if (!m_ptLtcDisabledOutputs->bMax7219) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	}

	if (!m_ptLtcDisabledOutputs->bWS28xx) {
		LtcDisplayRgb::Get()->Show(m_aTimeCode);
	}

	HandleInternalKeyEsc();
}

void McpButtons::HandleInternalTimeCodeStop(TLtcTimeCode &StartTimeCode) {
	displayEditTimeCode.HandleKey(m_nKey, StartTimeCode, m_aTimeCode);

	if (!m_ptLtcDisabledOutputs->bMax7219) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	}

	if (!m_ptLtcDisabledOutputs->bWS28xx) {
		LtcDisplayRgb::Get()->Show(m_aTimeCode);
	}

	HandleInternalKeyEsc();
}

void McpButtons::HandleInternalTimeCodeFps(TLtcTimeCode &StartTimeCode) {
	displayEditFps.HandleKey(m_nKey, StartTimeCode.nType);

	HandleInternalKeyEsc();
}

void McpButtons::HandleInternalKeyEsc() {
	if (m_nKey == INPUT_KEY_ESC) {
		Display::Get()->SetCursor(display::cursor::OFF);
		Display::Get()->SetCursorPos(0, 0);
		Display::Get()->ClearLine(1);
		Display::Get()->ClearLine(2);
		m_State = SOURCE_SELECT;
	}
}
