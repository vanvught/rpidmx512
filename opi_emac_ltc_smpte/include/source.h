/**
 * @file source.h
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SOURCE_H_
#define SOURCE_H_

#include "ltc.h"
#include "sourceconst.h"
#include "display.h"
#include "ntpclient.h"
#include "tcnetdisplay.h"

class Source {
public:
	static void Show(ltc::source ltcSource, bool bRunGpsTimeClient) {
		Display::Get()->ClearLine(4);
		Display::Get()->PutString(SourceConst::SOURCE[ltcSource]);

		if (ltcSource == ltc::source::SYSTIME) {
			Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->getCols() - 3U), 3);
			if (bRunGpsTimeClient) {
				Display::Get()->PutString("GPS");
			} else if ((NtpClient::Get()->GetStatus() != ntpclient::Status::FAILED) && (NtpClient::Get()->GetStatus() != ntpclient::Status::STOPPED)) {
				Display::Get()->PutString("NTP");
			} else if (HwClock::Get()->IsConnected()) {
				Display::Get()->PutString("RTC");
			}
		} else if (ltcSource == ltc::source::TCNET) {
			TCNetDisplay::Show();
		}
	}
};

#endif /* SOURCE_H_ */
