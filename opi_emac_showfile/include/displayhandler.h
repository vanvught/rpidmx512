/**
 * @file displayhandler.h
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

#ifndef DISPLAYHANDLER_H_
#define DISPLAYHANDLER_H_

#include <stdint.h>
#include <cassert>

#include "showfile.h"
#include "showfiledisplay.h"

#include "display.h"
#include "display7segment.h"

class DisplayHandler: public ShowFileDisplay {
public:
	DisplayHandler() {}
	~DisplayHandler() {}

	void ShowFileName(const char *pFileName, uint8_t nShow) {
		assert(pFileName != nullptr);

		if (pFileName[0] != 0) {
			Display::Get()->TextStatus(pFileName, nShow);
		} else {
			Display::Get()->TextStatus("No showfile", Display7SegmentMessage::ERROR_PLAYER);
		}
	}

	void ShowShowFileStatus() {
		Display::Get()->SetCursorPos(0, 6);

		switch (ShowFile::Get()->GetStatus()) {
			case ShowFileStatus::IDLE:
				Display::Get()->PutString("Idle     ");
				break;
			case ShowFileStatus::RUNNING:
				Display::Get()->PutString("Running  ");
				break;
			case ShowFileStatus::STOPPED:
				Display::Get()->PutString("Stopped  ");
				break;
			case ShowFileStatus::ENDED:
				Display::Get()->PutString("Ended    ");
				break;
			case ShowFileStatus::UNDEFINED:
			default:
				Display::Get()->PutString("No Status");
				break;
		}

		Display::Get()->SetCursorPos(10, 7);

		if (ShowFile::Get()->IsTFTPEnabled()) {
			Display::Get()->PutString(" [TFTP On]");
		} else 	if (ShowFile::Get()->GetDoLoop()) {
			Display::Get()->PutString(" [Looping]");
		} else {
			Display::Get()->PutString("          ");
		}
	}
};

#endif /* DISPLAYHANDLER_H_ */
