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

#include "console.h"

class DisplayUdfHandler: public ArtNetDisplay {
public:
	DisplayUdfHandler(ArtNetNode *pArtNetNode);
	~DisplayUdfHandler(void);

	void ShowShortName(const char *pShortName) {
		DisplayUdf::Get()->ShowNodeName(m_pArtNetNode);
		UpdateConsole();
	}

	void ShowLongName(const char *pLongName) {
		UpdateConsole();
	}

	void ShowUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
		UpdateConsole();
	}

	void ShowNetSwitch(uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
		UpdateConsole();
	}

	void ShowSubnetSwitch(uint8_t nAddress) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
		UpdateConsole();
	}

	void ShowMergeMode(uint8_t nPortIndex, TMerge tMerge) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
		UpdateConsole();
	}

	void ShowPortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol) {
		DisplayUdf::Get()->ShowUniverse(m_pArtNetNode);
		UpdateConsole();
	}

private:
	void UpdateConsole(void) {
		console_clear_top_row();
		m_pArtNetNode->Print();
	}

private:
	ArtNetNode *m_pArtNetNode;
};

#endif /* DISPLAYUDFHANDLER_H_ */
