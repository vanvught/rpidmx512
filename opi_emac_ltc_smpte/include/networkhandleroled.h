/**
 * @file networkhandleroled.h
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

#ifndef NETWORKHANDLEROLED_H_
#define NETWORKHANDLEROLED_H_

#include "display.h"
#include "display7segment.h"

#include "network.h"
#include "ntpclient.h"

class NetworkHandlerOled: public NetworkDisplay, public NtpClientDisplay {
public:
	NetworkHandlerOled() {
		s_pThis = this;
	}

	~NetworkHandlerOled() {
	}

	void ShowIp() {
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, IPSTR "/%d %c", IP2STR(Network::Get()->GetIp()), static_cast<int>(Network::Get()->GetNetmaskCIDR()), Network::Get()->GetAddressingMode());
	}

	void ShowNetMask() {
		ShowIp();
	}

	void ShowHostName() {
	}

	void ShowShutdown() {

	}

	// DHCP Client
	void ShowDhcpStatus(DhcpClientStatus nStatus) {
		switch (nStatus) {
		case DhcpClientStatus::IDLE:
			break;
		case DhcpClientStatus::RENEW:
			Display7Segment::Get()->Status(Display7SegmentMessage::INFO_DHCP);
			Display::Get()->ClearLine(3);
			Display::Get()->Printf(3, "DHCP renewing");
			break;
		case DhcpClientStatus::GOT_IP:
			Display7Segment::Get()->Status(Display7SegmentMessage::INFO_NONE);
			break;
		case DhcpClientStatus::FAILED:
			Display7Segment::Get()->Status(Display7SegmentMessage::ERROR_DHCP);
			break;
		default:
			break;
		}
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

	static NetworkHandlerOled *Get() {
		return s_pThis;
	}

private:
	static NetworkHandlerOled *s_pThis;
};

NetworkHandlerOled *NetworkHandlerOled::s_pThis = 0;

#endif /* NETWORKHANDLEROLED_H_ */
