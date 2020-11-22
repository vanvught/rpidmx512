/**
 * @file displayudfhandler.h
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

#ifndef DISPLAYUDFHANDLER_H_
#define DISPLAYUDFHANDLER_H_

#include <stdint.h>

#include "displayudf.h"

#include "artnetnode.h"
#include "artnet.h"

#include "artnetdisplay.h"
#include "lightsetdisplay.h"

#include "network.h"
#include "ntpclient.h"

class DisplayUdfHandler: public ArtNetDisplay, public LightSetDisplay, public NetworkDisplay, public NtpClientDisplay {
public:
	DisplayUdfHandler() {
	}
	~DisplayUdfHandler() {
	}

	void ShowShortName(__attribute__((unused)) const char *pShortName) {
		DisplayUdf::Get()->ShowNodeName(ArtNetNode::Get());
	}

	void ShowLongName(__attribute__((unused)) const char *pLongName) {
	}

	void ShowUniverseSwitch(__attribute__((unused))  uint8_t nPortIndex, __attribute__((unused))  uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(ArtNetNode::Get());
	}

	void ShowNetSwitch(__attribute__((unused))  uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(ArtNetNode::Get());
	}

	void ShowSubnetSwitch(__attribute__((unused))  uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(ArtNetNode::Get());
	}

	void ShowMergeMode(__attribute__((unused))  uint8_t nPortIndex, __attribute__((unused))  ArtNetMerge tMerge) {
		DisplayUdf::Get()->ShowUniverse(ArtNetNode::Get());
	}

	void ShowPortProtocol(__attribute__((unused))  uint8_t nPortIndex, __attribute__((unused))  TPortProtocol tPortProtocol) {
		DisplayUdf::Get()->ShowUniverse(ArtNetNode::Get());
	}

	void ShowDmxStartAddress() {
		DisplayUdf::Get()->ShowDmxStartAddress();
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

#endif /* DISPLAYUDFHANDLER_H_ */
