/**
 * @file displayudfhandler.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "artnetdisplay.h"

#include "artnetnode.h"

#include "displayudf.h"

#include "networkdisplay.h"

class DisplayUdfHandler: public ArtNetDisplay, NetworkDisplay {
public:
	DisplayUdfHandler(ArtNetNode *pArtNetNode);
	~DisplayUdfHandler(void);

	void ShowShortName(const char *pShortName) {
		DisplayUdf::Get()->ShowNodeName(m_pArtNetNode);
	}

	void ShowLongName(const char *pLongName) {}

	void ShowUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
	}

	void ShowNetSwitch(uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
	}

	void ShowSubnetSwitch(uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
	}

	void ShowMergeMode(uint8_t nPortIndex, TMerge tMerge) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
	}

	void ShowPortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
	}

	void ShowIp(void) {
		DisplayUdf::Get()->ShowIpAddress();
	}

	void ShowNetMask(void) {
		DisplayUdf::Get()->ShowNetmask();
	}

	void ShowHostName(void) {
		DisplayUdf::Get()->ShowHostName();
	}

private:
	ArtNetNode *m_pArtNetNode;
};

#endif /* DISPLAYUDFHANDLER_H_ */
