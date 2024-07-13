/**
 * @file showfiledisplay.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <cassert>

#include "showfile.h"
#include "showfiledisplay.h"

#include "display.h"

namespace showfile {
void display_filename(const char *pFileName, const uint32_t nShow) {
	assert(pFileName != nullptr);

	if (pFileName[0] != 0) {
		Display::Get()->TextStatus(pFileName, static_cast<uint8_t>(nShow));
	} else {
		Display::Get()->TextStatus("No showfile");
	}
}

void  display_status() {
	Display::Get()->SetCursorPos(0, 6);

	switch (ShowFile::Get()->GetStatus()) {
		case showfile::Status::IDLE:
			Display::Get()->PutString("Idle     ");
			break;
		case showfile::Status::PLAYING:
			Display::Get()->PutString("Running  ");
			break;
		case showfile::Status::STOPPED:
			Display::Get()->PutString("Stopped  ");
			break;
		case showfile::Status::ENDED:
			Display::Get()->PutString("Ended    ");
			break;
		case showfile::Status::RECORDING:
			Display::Get()->PutString("Recording ");
			break;
		case showfile::Status::UNDEFINED:
		default:
			Display::Get()->PutString("No Status");
			break;
	}

	Display::Get()->SetCursorPos(11, 7);

	if (ShowFile::Get()->IsTFTPEnabled()) {
		Display::Get()->PutString("[TFTP On]");
	} else if (ShowFile::Get()->GetDoLoop()) {
		Display::Get()->PutString("[Looping]");
	} else {
		Display::Get()->PutString("         ");
	}
}
}  // namespace showfile








