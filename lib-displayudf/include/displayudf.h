/**
 * @file displayudf.h
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

#ifndef DISPLAYUDF_H_
#define DISPLAYUDF_H_

#include <stdint.h>
#include <stdarg.h>

#include "display.h"

#include "artnetnode.h"
#include "e131bridge.h"

#include "network.h"

enum TDisplayUdfLabels {
	DISPLAY_UDF_LABEL_TITLE,
	DISPLAY_UDF_LABEL_BOARDNAME,
	DISPLAY_UDF_LABEL_IP,
	DISPLAY_UDF_LABEL_VERSION,
	DISPLAY_UDF_LABEL_UNIVERSE,
	DISPLAY_UDF_LABEL_AP,
	DISPLAY_UDF_LABEL_NODE_NAME,
	DISPLAY_UDF_LABEL_HOSTNAME,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_A,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_B,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_C,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_D,
	DISPLAY_UDF_LABEL_NETMASK,
	DISPLAY_UDF_LABEL_DMX_START_ADDRESS,
	DISPLAY_UDF_LABEL_DESTINATION_IP_PORT_A,
	DISPLAY_UDF_LABEL_DESTINATION_IP_PORT_B,
	DISPLAY_UDF_LABEL_DESTINATION_IP_PORT_C,
	DISPLAY_UDF_LABEL_DESTINATION_IP_PORT_D,
	DISPLAY_UDF_LABEL_UNKNOWN
};

#define DISPLAY_LABEL_MAX_ROWS		6

class DisplayUdf: public Display {
public:
	DisplayUdf();

	void SetTitle(const char *format, ...);

	void Show(ArtNetNode *pArtNetNode);
	void ShowNodeName(ArtNetNode *pArtNetNode);
	void ShowUniverse(ArtNetNode *pArtNetNode);
	void ShowDestinationIp(ArtNetNode *pArtNetNode);

	void Show(E131Bridge *pE131Bridge);

	// LightSet
	void ShowDmxStartAddress();

	// Network
	void ShowIpAddress();
	void ShowNetmask();
	void ShowHostName();
	void ShowDhcpStatus(DhcpClientStatus nStatus);
	void ShowShutdown();

	void Set(uint8_t nLine, enum TDisplayUdfLabels tLabel);

	uint8_t GetLabel(uint8_t nIndex) {
		if (nIndex < DISPLAY_UDF_LABEL_UNKNOWN) {
			return m_aLabels[nIndex];
		}

		return m_aLabels[0];
	}

	static DisplayUdf *Get() {
		return s_pThis;
	}

	void Show();

private:
	char m_aTitle[32];
	uint8_t m_aLabels[DISPLAY_UDF_LABEL_UNKNOWN];

	static DisplayUdf *s_pThis;
};

#endif /* DISPLAYUDF_H_ */
