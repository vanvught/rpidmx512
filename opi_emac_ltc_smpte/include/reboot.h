/**
 * @file reboot.h
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

#ifndef REBOOT_H_
#define REBOOT_H_

#include "reboothandler.h"

#include "ltc.h"

#include "remoteconfig.h"
#include "display.h"

#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"

#include "network.h"
#include "ntpclient.h"
#include "hwclock.h"

#include "tcnet.h"

#include "debug.h"

class Reboot: public RebootHandler {
public:
	Reboot(ltc::source tSource) :m_tSource(tSource) {
	}

	~Reboot(void) {
	}

	void Run(void) {
		DEBUG_ENTRY

		switch (m_tSource) {
		case ltc::source::TCNET:
			TCNet::Get()->Stop();
			break;
		default:
			break;
		}

		if ((NtpClient::Get()->GetStatus() != NtpClientStatus::FAILED) && (NtpClient::Get()->GetStatus() != NtpClientStatus::STOPPED)) {
			HwClock::Get()->SysToHc();
		}

		if (LtcOutputs::Get()->IsActiveMax7219()) {
			LtcDisplayMax7219::Get()->Init(2); // TODO WriteChar
		}

		if (LtcOutputs::Get()->IsActiveWS28xx()) {
			LtcDisplayWS28xx::Get()->WriteChar('-');
		}

		if (!RemoteConfig::Get()->IsReboot()) {
			DEBUG_PUTS("");

			Display::Get()->SetSleep(false);

			while (SpiFlashStore::Get()->Flash())
				;

			Network::Get()->Shutdown();

			printf("Rebooting ...\n");

			Display::Get()->Cls();
			Display::Get()->TextStatus("Rebooting ...", Display7SegmentMessage::INFO_REBOOTING);
		}

		DEBUG_ENTRY
	}

private:
	ltc::source m_tSource;
};

#endif /* REBOOT_H_ */
