/**
 * @file displayudf.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cassert>

#include "displayudf.h"
#include "display.h"

#include "lightset.h"
#include "network.h"

#include "hardware.h"
#include "firmwareversion.h"

#include "debug.h"

using namespace displayudf;

DisplayUdf *DisplayUdf::s_pThis = nullptr;

DisplayUdf::DisplayUdf() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		m_aLabels[i] = static_cast<uint8_t>(i + 1);
	}

	DEBUG_EXIT
}

void DisplayUdf::SetTitle(const char *format, ...) {
	va_list arp;
	va_start(arp, format);

	const auto i = vsnprintf(m_aTitle, sizeof(m_aTitle) / sizeof(m_aTitle[0]) - 1, format, arp);

	va_end(arp);

	m_aTitle[i] = '\0';

	DEBUG_PUTS(m_aTitle);
}

void DisplayUdf::Set(uint32_t nLine, Labels label) {
	if (!((nLine > 0) && (nLine <= LABEL_MAX_ROWS))) {
		return;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (m_aLabels[i] == static_cast<uint8_t>(nLine)) {
			m_aLabels[i] = m_aLabels[static_cast<uint32_t>(label)];
			break;
		}
	}

	m_aLabels[static_cast<uint32_t>(label)] = static_cast<uint8_t>(nLine);
}

void DisplayUdf::Show() {
#if defined (NODE_ARTNET)
	ShowArtNetNode();
#elif defined (NODE_E131)
	ShowE131Bridge();
#elif defined (NODE_NODE)
	ShowNode();
#endif

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (m_aLabels[i] > LABEL_MAX_ROWS) {
			m_aLabels[i] = 0xFF;
		}

		DEBUG_PRINTF("m_aLabels[%d]=%d", i, m_aLabels[i]);
	}

	ClearEndOfLine();
	Write(m_aLabels[static_cast<uint32_t>(Labels::TITLE)], m_aTitle);
	uint8_t nHwTextLength;
	ClearEndOfLine();
	Write(m_aLabels[static_cast<uint32_t>(Labels::BOARDNAME)], Hardware::Get()->GetBoardName(nHwTextLength));
	ClearEndOfLine();
	Printf(m_aLabels[static_cast<uint32_t>(Labels::VERSION)], "Firmware V%.*s", firmwareversion::length::SOFTWARE_VERSION, FirmwareVersion::Get()->GetVersion()->SoftwareVersion);

#if defined (RDM_RESPONDER)
	ShowDmxStartAddress();
#endif

#if !defined (NO_EMAC)
	ShowIpAddress();
	ShowGatewayIp();
	ShowNetmask();
	ShowHostName();
#endif
}
