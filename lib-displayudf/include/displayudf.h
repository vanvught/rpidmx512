/**
 * @file displayudf.h
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

#ifndef DISPLAYUDF_H_
#define DISPLAYUDF_H_

#include <stdint.h>
#include <stdarg.h>

#include "display.h"

#include "artnetnode.h"
#include "e131bridge.h"

enum TDisplayUdfLabels {
	DISPLAY_UDF_LABEL_TITLE,
	DISPLAY_UDF_LABEL_BOARDNAME,
	DISPLAY_UDF_LABEL_IP,
	DISPLAY_UDF_LABEL_NETMASK,
	DISPLAY_UDF_LABEL_UNIVERSE,
	DISPLAY_UDF_LABEL_AP,
	DISPLAY_UDF_LABEL_NODE_NAME,
	DISPLAY_UDF_LABEL_HOSTNAME,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_A,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_B,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_C,
	DISPLAY_UDF_LABEL_UNIVERSE_PORT_D,
	DISPLAY_UDF_LABEL_UNKNOWN
};

#define DISPLAY_LABEL_MAX_ROWS		6

class DisplayUdf: public Display {
public:
	DisplayUdf(void);
	~DisplayUdf(void);

	void SetTitle(const char *format, ...);

	void Show(ArtNetNode *pArtNetNode);
	void ShowNodeName(ArtNetNode *pArtNetNode);
	void ShowUniverse(ArtNetNode *pArtNetNode);

	void Show(E131Bridge *pE131Bridge);

	void ShowIpAddress(void);

	void Set(uint8_t nLine, enum TDisplayUdfLabels tLabel);

	static DisplayUdf* Get(void) {
		return s_pThis;
	}

private:
	void Show(void);

private:
	uint8_t m_aTitle[32];
	uint8_t m_aLabels[DISPLAY_UDF_LABEL_UNKNOWN];

	static DisplayUdf *s_pThis;
};

#endif /* DISPLAYUDF_H_ */
