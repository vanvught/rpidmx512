/**
 * @file widgetsniffer.cpp
 *
 */
/* Copyright (C) 2015-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstddef>
#include <stdbool.h>

#include "widget.h"
#include "widgetmonitor.h"

#include "hardware.h"

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "usb.h"

#include "debug.h"

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

#define	SNIFFER_PACKET			0x81	///< Label
#define	SNIFFER_PACKET_SIZE  	200		///< Packet size
#define CONTROL_MASK			0x00	///< If the high bit is set, this is a data byte, otherwise it's a control byte
#define DATA_MASK				0x80	///< If the high bit is set, this is a data byte, otherwise it's a control byte

using namespace widget;
using namespace widgetmonitor;

void Widget::UsbSendPackage(const uint8_t *pData, uint16_t Start, uint16_t nDataLength) {
	uint32_t i;

	if (nDataLength < (SNIFFER_PACKET_SIZE / 2)) {
		SendHeader(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

		for (i = 0; i < nDataLength; i++) {
			usb_send_byte(DATA_MASK);
			usb_send_byte(pData[i + Start]);
		}

		for (i = nDataLength; i < SNIFFER_PACKET_SIZE / 2; i++) {
			usb_send_byte(CONTROL_MASK);
			usb_send_byte(0x02);
		}

		SendFooter();
	} else {
		SendHeader(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

		for (i = 0; i < SNIFFER_PACKET_SIZE / 2; i++) {
			usb_send_byte(DATA_MASK);
			usb_send_byte(pData[i + Start]);
		}

		SendFooter();

		UsbSendPackage(pData, static_cast<uint16_t>(Start + SNIFFER_PACKET_SIZE / 2), static_cast<uint16_t>(nDataLength - SNIFFER_PACKET_SIZE / 2));
	}
}

bool Widget::UsbCanSend() {
	const auto nMicros = Hardware::Get()->Micros();

	while (!usb_can_write() && (Hardware::Get()->Micros() - nMicros < 1000)) {
	}

	if (!usb_can_write()) {
		WidgetMonitor::Line(MonitorLine::INFO, "!Failed! Cannot send to host");
		return false;
	}

	return true;
}

/**
 * This function is called from Run
 */
void Widget::SnifferDmx() {
	if ((GetMode() != Mode::RDM_SNIFFER) || !UsbCanSend()) {
		return;
	}

	const auto *pDmxData = Dmx::GetDmxChanged(0);

	if (pDmxData == nullptr) {
		return;
	}

	const auto *pDmxStatistics = reinterpret_cast<const struct Data *>(pDmxData);
	const auto nDataLength = pDmxStatistics->Statistics.nSlotsInPacket + 1;

	if (!UsbCanSend()) {
		return;
	}

	WidgetMonitor::Line(MonitorLine::INFO, "Send DMX data to HOST -> %d", nDataLength);
	UsbSendPackage(pDmxData, 0, static_cast<uint16_t>(nDataLength));
}

/**
 * This function is called from Run
 */
void Widget::SnifferRdm() {
	if ((GetMode() != Mode::RDM_SNIFFER) || !UsbCanSend()) {
		return;
	}

	const auto *pRdmData = Rdm::Receive(0);

	if (pRdmData == nullptr) {
		return;
	}

	uint8_t nMessageLength = 0;

	if (pRdmData[0] == E120_SC_RDM) {
		const auto *p = reinterpret_cast<const struct TRdmMessage *>(pRdmData);
		nMessageLength = static_cast<uint8_t>(p->message_length + 2);
		switch (p->command_class) {
		case E120_DISCOVERY_COMMAND:
			m_RdmStatistics.nDiscoveryPackets++;
			break;
		case E120_DISCOVERY_COMMAND_RESPONSE:
			m_RdmStatistics.nDiscoveryResponsePackets++;
			break;
		case E120_GET_COMMAND:
			m_RdmStatistics.nGetRequests++;
			break;
		case E120_SET_COMMAND:
			m_RdmStatistics.nSetRequests++;
			break;
		default:
			break;
		}
	} else if (pRdmData[0] == 0xFE) {
		m_RdmStatistics.nDiscoveryResponsePackets++;
		nMessageLength = 24;
	}

	if (!UsbCanSend()) {
		return;
	}

	WidgetMonitor::Line(MonitorLine::INFO, "Send RDM data to HOST");
	UsbSendPackage(pRdmData, 0, nMessageLength);
}

void Widget::SnifferFillTransmitBuffer() {
	if (!UsbCanSend()) {
		return;
	}

	int i = 256;

	while (i--) {
		if (!UsbCanSend()) {
			return;
		}
		usb_send_byte(0);
	}
}
