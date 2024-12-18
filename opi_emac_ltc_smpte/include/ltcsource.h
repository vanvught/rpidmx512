/**
 * @file ltcsource.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCSOURCE_H_
#define LTCSOURCE_H_

#include "ltc.h"
#include "ltcsourceconst.h"
#include "display.h"
#include "gps.h"
#include "net/apps/ntp_client.h"
#include "tcnetdisplay.h"

namespace ltc {
namespace source {
inline static void show(ltc::Source ltcSource, bool bRunGpsTimeClient) {
	Display::Get()->TextStatus(LtcSourceConst::NAME[static_cast<uint32_t>(ltcSource)]);

	if (ltcSource == ltc::Source::SYSTIME) {
		Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 3U), 3);
		if (bRunGpsTimeClient) {
			GPS::Get()->Display(GPS::Get()->GetStatus());
		} else if ((ntp_client_get_status() != ::ntp::Status::FAILED) && (ntp_client_get_status() != ::ntp::Status::STOPPED)  && (ntp_client_get_status() != ::ntp::Status::DISABLED)) {
			Display::Get()->PutString("NTP");
		} else if (HwClock::Get()->IsConnected()) {
			Display::Get()->PutString("RTC");
		}
	} else if (ltcSource == ltc::Source::TCNET) {
		tcnet::display::show();
	}
}
}  // namespace source
}  // namespace ltc

#endif /* SOURCE_H_ */
