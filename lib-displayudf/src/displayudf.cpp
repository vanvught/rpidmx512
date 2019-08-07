/**
 * @file displayudf.cpp
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

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "displayudf.h"
#include "display.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

DisplayUdf *DisplayUdf::s_pThis = 0;

DisplayUdf::DisplayUdf(void): Display(DISPLAY_SSD1306) {
	s_pThis = this;

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		m_aLabels[i] = i + 1;
	}
}

DisplayUdf::~DisplayUdf(void) {
}

void DisplayUdf::SetTitle(const char *format, ...) {
	va_list arp;
	va_start(arp, format);

	const uint32_t i = vsnprintf((char *)m_aTitle, sizeof(m_aTitle) / sizeof(m_aTitle[0]) - 1, format, arp);

	va_end(arp);

	m_aTitle[i] = '\0';

	DEBUG_PUTS(m_aTitle);
}

void DisplayUdf::Show(void) {
	DEBUG1_ENTRY

	uint8_t nHwTextLength;

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (m_aLabels[i] > DISPLAY_LABEL_MAX_ROWS) {
			m_aLabels[i] = 0xFF;
		}

		DEBUG_PRINTF("m_aLabels[%d]=%d", i, m_aLabels[i]);
	}

	for (unsigned i = 1; i < DISPLAY_LABEL_MAX_ROWS; i++) {
		ClearLine(i);
	}

	Write(m_aLabels[DISPLAY_UDF_LABEL_TITLE], (const char *)m_aTitle);
	Write(m_aLabels[DISPLAY_UDF_LABEL_BOARDNAME], Hardware::Get()->GetBoardName(nHwTextLength));
	Printf(m_aLabels[DISPLAY_UDF_LABEL_NETMASK], "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	Write(m_aLabels[DISPLAY_UDF_LABEL_HOSTNAME], Network::Get()->GetHostName());
	ShowIpAddress();

	DEBUG1_EXIT
}

void DisplayUdf::ShowIpAddress(void) {
	Printf(m_aLabels[DISPLAY_UDF_LABEL_IP], "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), Network::Get()->IsDhcpKnown() ? (Network::Get()->IsDhcpUsed() ? 'D' : 'S') : ' ');
}

void DisplayUdf::Set(uint8_t nLine, enum TDisplayUdfLabels tLabel) {
	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (m_aLabels[i] == nLine) {
			m_aLabels[i] = m_aLabels[tLabel];
			break;
		}
	}

	m_aLabels[tLabel] = nLine;
}
