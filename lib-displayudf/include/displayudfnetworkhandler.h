/**
 * @file displayudfnetworkhandler.h
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

#ifndef DISPLAYUDFNETWORKHANDLER_H_
#define DISPLAYUDFNETWORKHANDLER_H_

#include "displayudf.h"

#include "network.h"
#include "ntpclient.h"

class DisplayUdfNetworkHandler: public NetworkDisplay, public NtpClientDisplay {
public:
	DisplayUdfNetworkHandler() {
	}
	~DisplayUdfNetworkHandler() {
	}

	void ShowIp() {
		DisplayUdf::Get()->ShowIpAddress();
	}

	void ShowNetMask() {
		DisplayUdf::Get()->ShowNetmask();
	}

	void ShowHostName() {
		DisplayUdf::Get()->ShowHostName();
	}

	void ShowShutdown() {
		DisplayUdf::Get()->ShowShutdown();
	}

	// DHCP Client
	void ShowDhcpStatus(DhcpClientStatus nStatus) {
		DisplayUdf::Get()->ShowDhcpStatus(nStatus);
	}

	// NTP Client
	void ShowNtpClientStatus(NtpClientStatus nStatus) {
		if (nStatus == NtpClientStatus::IDLE) {
			Display::Get()->TextStatus("NTP Client", Display7SegmentMessage::INFO_NTP);
			return;
		}

		if (nStatus == NtpClientStatus::FAILED) {
			Display::Get()->TextStatus("Error: NTP", Display7SegmentMessage::ERROR_NTP);
			return;
		}
	}
};

#endif /* DISPLAYUDFNETWORKHANDLER_H_ */
