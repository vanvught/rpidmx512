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

#include "networkdisplay.h"

#include "display.h"

#include "networkdisplay.h"
#include "network.h"

class NetworkHandlerOled: public NetworkDisplay {
public:
	NetworkHandlerOled(void) {}
	~NetworkHandlerOled(void) {}

	void ShowIp(void) {
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, IPSTR "/%d %c",
				IP2STR(Network::Get()->GetIp()),
				static_cast<int>(Network::Get()->GetNetmaskCIDR()),
				Network::Get()->IsDhcpKnown() ? (Network::Get()->IsDhcpUsed() ? 'D' : 'S') : ' ');
	}

	void ShowNetMask(void) {
		ShowIp();
	}

	void ShowHostName(void) {};
};

#endif /* NETWORKHANDLEROLED_H_ */
