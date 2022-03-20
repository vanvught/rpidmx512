/**
 * @file widget.cpp
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
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
#include <cassert>

#include "widget.h"
#include "widgetconfiguration.h"
#include "widgetmonitor.h"

#include "hardware.h"

#include "dmx.h"
#include "rdm.h"
#include "rdmdevice.h"
#include "rdm_e120.h"

#include "usb.h"

enum {
	GET_WIDGET_PARAMS = 3,					///< Get Widget Parameters Request
	GET_WIDGET_PARAMS_REPLY = 3,			///< Get Widget Parameters Reply
	SET_WIDGET_PARAMS = 4,					///< Set Widget Parameters Request
	RECEIVED_DMX_PACKET = 5,				///< Received DMX Packet
	OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST = 6,///< Output Only Send DMX Packet Request
	SEND_RDM_PACKET_REQUEST = 7,			///< Send RDM Packet Request
	RECEIVE_DMX_ON_CHANGE = 8,				///< Receive DMX on Change
	RECEIVED_DMX_COS_TYPE = 9,				///< Received DMX Change Of State Packet
	GET_WIDGET_SN_REQUEST = 10,				///< Get Widget Serial Number Request
	GET_WIDGET_SN_REPLY = 10,				///< Get Widget Serial Number Reply
	SEND_RDM_DISCOVERY_REQUEST = 11,		///< Send RDM Discovery Request
	RDM_TIMEOUT = 12,						///< https://github.com/OpenLightingProject/ola/blob/master/plugins/usbpro/EnttecUsbProWidget.cpp#L353
	MANUFACTURER_LABEL = 77,				///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
	GET_WIDGET_NAME_LABEL = 78				///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
};

using namespace widget;
using namespace widgetmonitor;
using namespace dmx;

Widget *Widget::s_pThis = nullptr;

Widget::Widget() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	usb_init();
}

/*
 * Widget LABELs
 */

/**
 *
 * Get Widget Parameters Reply (Label=3 \ref GET_WIDGET_PARAMS_REPLY)
 * The Widget sends this message to the PC in response to the Get Widget Parameters request.
 */
void Widget::GetParamsReply() {
	WidgetMonitor::Line(MonitorLine::INFO, "GET_WIDGET_PARAMS_REPLY");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	TWidgetConfiguration widgetConfiguration;
	WidgetConfiguration::Get(&widgetConfiguration);
	SendMessage(GET_WIDGET_PARAMS_REPLY, reinterpret_cast<uint8_t *>(&widgetConfiguration), sizeof(struct TWidgetConfiguration));
}

/**
 *
 * Set Widget Parameters Request (Label=4 SET_WIDGET_PARAMS)
 * This message sets the Widget configuration. The Widget configuration is preserved when the Widget loses power.
 *
 */
void Widget::SetParams() {
	TWidgetConfiguration widgetConfiguration;

	WidgetMonitor::Line(MonitorLine::INFO, "SET_WIDGET_PARAMS");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	SetPortDirection(0, PortDirection::INP, false);

	widgetConfiguration.nBreakTime = m_aData[2];
	widgetConfiguration.nMabTime = m_aData[3];
	widgetConfiguration.nRefreshRate = m_aData[4];
	WidgetConfiguration::Store(&widgetConfiguration);

	SetPortDirection(0, PortDirection::INP, true);

	m_nReceivedDmxPacketStartMillis = Hardware::Get()->Millis();
}

/**
 *
 * This function is called from Run
 *
 * Received DMX Packet (Label=5 \ref RECEIVED_DMX_PACKET)
 *
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode (\ref receive_dmx_on_change) is 'Send always' (\ref SEND_ALWAYS).
 */
void Widget::ReceivedDmxPacket() {
	if (m_tMode == widget::Mode::RDM_SNIFFER) {
		return;
	}

	if (m_isRdmDiscoveryRunning
			|| (PortDirection::INP != GetPortDirection())
			|| (SendState::ON_DATA_CHANGE_ONLY == m_tReceiveDmxOnChange)) {
		return;
	}

	const auto *pDmxData = GetDmxAvailable(0);

	if (pDmxData == nullptr) {
		return;
	}

	const auto nMillis = Hardware::Get()->Millis();

	if (nMillis - m_nReceivedDmxPacketStartMillis < m_nReceivedDmxPacketPeriodMillis) {
		return;
	}

	m_nReceivedDmxPacketStartMillis = nMillis;
	m_nReceivedDmxPacketCount++;

	const auto *pDmxStatistics = reinterpret_cast<const struct Data *>(pDmxData);
	const auto nLength = pDmxStatistics->Statistics.nSlotsInPacket + 1;

	WidgetMonitor::Line(MonitorLine::LABEL, "RECEIVED_DMX_PACKET");
	WidgetMonitor::Line(MonitorLine::INFO, "Send DMX data to HOST, %d", nLength);
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	SendHeader(RECEIVED_DMX_PACKET, static_cast<uint16_t>(nLength + 1));
	usb_send_byte(0); 	// DMX Receive status
	SendData(pDmxData, static_cast<uint16_t>(nLength));
	SendFooter();
}

/**
 *
 * This function is called from Run
 *
 * Received DMX Packet (Label=5 RECEIVED_DMX_PACKET)
 *
 * The Widget sends this message to the PC unsolicited, whenever the Widget receives a DMX or RDM packet from the DMX port,
 * and the Receive DMX on Change mode is 'Send always' (SEND_ALWAYS).
 */
void Widget::ReceivedRdmPacket() {
	if ((m_tMode == widget::Mode::DMX) || (m_tMode == widget::Mode::RDM_SNIFFER)
			|| (m_tReceiveDmxOnChange == SendState::ON_DATA_CHANGE_ONLY)) {
		return;
	}

	const auto *pRdmData = Rdm::Receive(0);

	if (pRdmData == nullptr) {
		return;
	}

	uint8_t nMessageLength = 0;

	if (pRdmData[0] == E120_SC_RDM) {
		const auto *p = reinterpret_cast<const struct TRdmMessage *>(pRdmData);
		const auto command_class = p->command_class;
		nMessageLength = static_cast<uint8_t>(p->message_length + 2);

		WidgetMonitor::Line(MonitorLine::INFO, "Send RDM data to HOST, l:%d", nMessageLength);
		WidgetMonitor::Line(MonitorLine::STATUS, "RECEIVED_RDM_PACKET SC:0xCC");

		SendHeader(RECEIVED_DMX_PACKET, static_cast<uint32_t>(1 + nMessageLength));
		usb_send_byte(0); 	// RDM Receive status
		SendData(pRdmData, nMessageLength);
		SendFooter();

		const auto param_id = static_cast<uint16_t>((p->param_id[0] << 8) + p->param_id[1]);

		if ((command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (param_id != E120_DISC_MUTE)) {
			RdmTimeOutMessage();
		} else {
			m_nSendRdmPacketStartMillis = 0;
		}
	} else if (pRdmData[0] == 0xFE) {
		nMessageLength = 24;

		WidgetMonitor::Line(MonitorLine::INFO, "Send RDM data to HOST, l:%d", nMessageLength);
		WidgetMonitor::Line(MonitorLine::STATUS, "RECEIVED_RDM_PACKET SC:0xFE");

		SendHeader(RECEIVED_DMX_PACKET, static_cast<uint32_t>(1 + nMessageLength));
		usb_send_byte(0); 	// RDM Receive status
		SendData(pRdmData, nMessageLength);
		SendFooter();

		RdmTimeOutMessage();
	}

	WidgetMonitor::RdmData(MonitorLine::RDM_DATA, nMessageLength, pRdmData, false);
}

/**
 *
 * Output Only Send DMX Packet Request (label = 6 \ref OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST)
 *
 * This message requests the Widget to periodically send a DMX packet out of the Widget DMX port
 * at the configured DMX output rate. This message causes the widget to leave the DMX port direction
 * as output after each DMX packet is sent, so no DMX packets will be received as a result of this
 * request.
 *
 * The periodic DMX packet output will stop and the Widget DMX port direction will change to input
 * when the Widget receives any request message other than the Output Only Send DMX Packet
 * request, or the Get Widget Parameters request.
 *
 * @param data_length DMX data to send, beginning with the start code.
 */
void Widget::SendDmxPacketRequestOutputOnly(uint16_t nDataLength) {
	if (m_nSendRdmPacketStartMillis != 0) {
		return;
	}

	WidgetMonitor::Line(MonitorLine::INFO, "OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	Dmx::SetPortDirection(0, PortDirection::OUTP, false);
	Dmx::SetSendData(0, m_aData, nDataLength);
	Dmx::SetPortDirection(0, PortDirection::OUTP, true);
}

/**
 *
 * Send RDM Packet Request (label = 7 SEND_RDM_PACKET_REQUEST)
 *
 * This message requests the Widget to send an RDM packet out of the Widget DMX port, and then
 * change the DMX port direction to input, so that RDM or DMX packets can be received.
 *
 * @param data_length RDM data to send, beginning with the start code.
 */
void Widget::SendRdmPacketRequest(uint16_t nDataLength) {
	WidgetMonitor::Line(MonitorLine::INFO, "SEND_RDM_PACKET_REQUEST");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	const auto *pData = reinterpret_cast<const struct TRdmMessage *>(m_aData);

	m_isRdmDiscoveryRunning = (pData->command_class == E120_DISCOVERY_COMMAND);

	Rdm::SendRaw(0, m_aData, nDataLength);

	m_nSendRdmPacketStartMillis = Hardware::Get()->Millis();

	WidgetMonitor::RdmData(MonitorLine::RDM_DATA, nDataLength, m_aData, true);
}

/**
 *
 * This function is called from Run
 *
 */
void Widget::RdmTimeout() {
	if (m_tMode == widget::Mode::RDM_SNIFFER) {
		return;
	}

	if (m_nSendRdmPacketStartMillis == 0) {
		return;
	}

	if (Hardware::Get()->Millis() - m_nSendRdmPacketStartMillis < 1000U) {	// 1 second
		return;
	}

	RdmTimeOutMessage();	// Send message to host Label=12 RDM_TIMEOUT
	m_nSendRdmPacketStartMillis = 0;
}

/**
 *
 * Receive DMX on Change (label = 8 \ref RECEIVE_DMX_ON_CHANGE)
 *
 * This message requests the Widget send a DMX packet to the PC only when the DMX values change
 * on the input port.
 *
 * By default the widget will always send, if you want to send on change it must be enabled by sending
 * this message.
 *
 * This message also reinitializes the DMX receive processing, so that if change of state reception is
 * selected, the initial received DMX data is cleared to all zeros.
 *
 */
void Widget::ReceiveDmxOnChange() {
	WidgetMonitor::Line(MonitorLine::INFO, "RECEIVE_DMX_ON_CHANGE");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	m_tReceiveDmxOnChange = static_cast<SendState>(m_aData[0]);

	Dmx::SetPortDirection(0, PortDirection::INP, false);
	Dmx::ClearData(0);
	Dmx::SetPortDirection(0, PortDirection::INP, true);

	m_nReceivedDmxPacketStartMillis = Hardware::Get()->Millis();
}

/**
 *
 * Received DMX Change Of State Packet (Label = 9 \ref RECEIVED_DMX_COS_TYPE)
 *
 * The Widget sends one or more instances of this message to the PC unsolicited, whenever the
 * Widget receives a changed DMX packet from the DMX port, and the Receive DMX on Change
 * mode (\ref receive_dmx_on_change) is 'Send on data change only' (\ref SEND_ON_DATA_CHANGE_ONLY).
 */
void Widget::ReceivedDmxChangeOfStatePacket() {
	if (m_tMode == widget::Mode::RDM_SNIFFER) {
		return;
	}

	if (m_isRdmDiscoveryRunning
			|| (PortDirection::INP != GetPortDirection())
			|| (SendState::ALWAYS == m_tReceiveDmxOnChange)) {
		return;
	}

	if (nullptr != Dmx::GetDmxChanged(0)) {
		WidgetMonitor::Line(MonitorLine::INFO, "RECEIVED_DMX_COS_TYPE");
		WidgetMonitor::Line(MonitorLine::STATUS, nullptr);
		// TODO widget_received_dmx_change_of_state_packet
		WidgetMonitor::Line(MonitorLine::INFO, "Sent changed DMX data to HOST");
	}
}

/**
 *
 * Get Widget Serial Number Reply (Label = 10 GET_WIDGET_PARAMS_REPLY)
 *
 */
void Widget::GetSnReply() {
	WidgetMonitor::Line(MonitorLine::INFO, "GET_WIDGET_PARAMS_REPLY");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	Dmx::SetPortDirection(0, PortDirection::INP, false);

	SendMessage(GET_WIDGET_SN_REPLY, GetSN(), DEVICE_SN_LENGTH);

	Dmx::SetPortDirection(0, PortDirection::INP, true);

	m_nReceivedDmxPacketStartMillis = Hardware::Get()->Millis();
}

/**
 *
 * Send RDM Discovery Request (Label=11 SEND_RDM_DISCOVERY_REQUEST)
 *
 * This message requests the Widget to send an RDM Discovery Request packet out of the Widget
 * DMX port, and then receive an RDM Discovery Response.
 */
void Widget::SendRdmDiscoveryRequest(uint16_t nDataLength) {
	WidgetMonitor::Line(MonitorLine::INFO, "SEND_RDM_DISCOVERY_REQUEST");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	Rdm::SendRaw(0, m_aData, nDataLength);

	m_isRdmDiscoveryRunning = true;
	m_nSendRdmPacketStartMillis = Hardware::Get()->Millis();

	WidgetMonitor::RdmData(MonitorLine::RDM_DATA, nDataLength, m_aData, true);
}

/**
 *
 * See https://github.com/OpenLightingProject/ola/blob/master/plugins/usbpro/EnttecUsbProWidget.cpp#L353
 *
 * (Label=12 RDM_TIMEOUT)
 *
 */
void Widget::RdmTimeOutMessage() {
	const auto nMessageLength = 0;

	WidgetMonitor::Line(MonitorLine::INFO, "Send RDM data to HOST, l:%d", nMessageLength);
	WidgetMonitor::Line(MonitorLine::STATUS, "RDM_TIMEOUT");

	SendHeader(RDM_TIMEOUT, nMessageLength);
	SendFooter();

	m_isRdmDiscoveryRunning = false;
	m_nSendRdmPacketStartMillis = 0;
}

/**
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Manufacturer.2C_Label_.3D_77.2C_no_data
 *
 * Get Widget Manufacturer Reply (Label = 77  MANUFACTURER_LABEL)
 */
void Widget::GetManufacturerReply() {
	WidgetMonitor::Line(MonitorLine::INFO, "MANUFACTURER_LABEL");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	TRDMDeviceInfoData tManufacturerName;
	GetManufacturerName(&tManufacturerName);

	TRDMDeviceInfoData tManufacturerId;
	GetManufacturerId(&tManufacturerId);

	Dmx::SetPortDirection(0, PortDirection::INP, false);

	SendHeader(MANUFACTURER_LABEL, static_cast<uint32_t>(tManufacturerId.length + tManufacturerName.length));
	SendData(reinterpret_cast<uint8_t *>(tManufacturerId.data), tManufacturerId.length);
	SendData(reinterpret_cast<uint8_t *>(tManufacturerName.data), tManufacturerName.length);
	SendFooter();

	Dmx::SetPortDirection(0, PortDirection::INP, true);

	m_nReceivedDmxPacketStartMillis = Hardware::Get()->Millis();
}

/**
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Name.2C_Label_.3D_78.2C_no_data
 *
 * Get Widget Name Reply (Label = 78 GET_WIDGET_NAME_LABEL)
 */
void Widget::GetNameReply() {
	WidgetMonitor::Line(MonitorLine::INFO, "GET_WIDGET_NAME_LABEL");
	WidgetMonitor::Line(MonitorLine::STATUS, nullptr);

	TRDMDeviceInfoData widgetLabel;
	GetLabel(&widgetLabel);

	TWidgetConfigurationData widgetTypeId;
	WidgetConfiguration::GetTypeId(&widgetTypeId);

	Dmx::SetPortDirection(0, PortDirection::INP, false);

	SendHeader(GET_WIDGET_NAME_LABEL, static_cast<uint32_t>(widgetTypeId.nLength + widgetLabel.length));
	SendData(widgetTypeId.pData, widgetTypeId.nLength);
	SendData(reinterpret_cast<uint8_t *>(widgetLabel.data), widgetLabel.length);
	SendFooter();

	Dmx::SetPortDirection(0, PortDirection::INP, true);

	m_nReceivedDmxPacketStartMillis = Hardware::Get()->Millis();
}

/**
 *
 * Read bytes from host
 *
 * This function is called from Run
 */
void Widget::ReceiveDataFromHost() {
	if (usb_read_is_byte_available()) {
		const auto nByte = usb_read_byte();

		if (static_cast<uint8_t>(Amf::START_CODE) == nByte) {
			const auto nLabel = usb_read_byte();
			const auto nLsb = usb_read_byte();
			const auto nMsb = usb_read_byte();
			const auto nDataLength = static_cast<uint16_t>((nMsb << 8) | nLsb);

			uint32_t i;

			for (i = 0; i < nDataLength; i++) {
				m_aData[i] = usb_read_byte();
			}

			while ((static_cast<uint8_t>(Amf::END_CODE) != usb_read_byte()) && (i++ < (sizeof(m_aData) / sizeof(m_aData[0]))))
				;

			WidgetMonitor::Line(MonitorLine::LABEL, "L:%d:%d(%d)", nLabel, nDataLength, i);

			switch (nLabel) {
			case GET_WIDGET_PARAMS:
				GetParamsReply();
				break;
			case GET_WIDGET_SN_REQUEST:
				GetSnReply();
				break;
			case SET_WIDGET_PARAMS:
				SetParams();
				break;
			case GET_WIDGET_NAME_LABEL:
				GetNameReply();
				break;
			case MANUFACTURER_LABEL:
				GetManufacturerReply();
				break;
			case OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST:
				SendDmxPacketRequestOutputOnly(nDataLength);
				break;
			case RECEIVE_DMX_ON_CHANGE:
				ReceiveDmxOnChange();
				break;
			case SEND_RDM_PACKET_REQUEST:
				SendRdmPacketRequest(nDataLength);
				break;
			case SEND_RDM_DISCOVERY_REQUEST:
				SendRdmDiscoveryRequest(nDataLength);
				break;
			default:
				break;
			}
		}
	}
}
