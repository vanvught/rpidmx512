/**
 * @file tcnetdisplay.cpp
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

#include "tcnetdisplay.h"

#include "tcnet.h"
#include "display.h"

#include "ltcdisplayrgb.h"

static constexpr char sFps[4][6] = { " T24", " T25", " T29", " T30" };

void TCNetDisplay::Show() {
	Display::Get()->SetCursorPos(6,3);

	Display::Get()->PutChar('L');
	Display::Get()->PutChar(TCNet::GetLayerName(TCNet::Get()->GetLayer()));

	if (TCNet::Get()->GetUseTimeCode()) {
		Display::Get()->PutString(" TC ");
	} else {
		Display::Get()->PutString(sFps[TCNet::Get()->GetTimeCodeType()]);
	}

	char Info[9] = "Layer  ";
	Info[6] = TCNet::GetLayerName(TCNet::Get()->GetLayer());
	LtcDisplayRgb::Get()->ShowInfo(Info);
}
