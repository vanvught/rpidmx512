/**
 * @file widget.h
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
 *
 */
/* Copyright (C) 2015-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WIDGET_H_
#define WIDGET_H_

#include <cstdint>

#include "dmx.h"
#include "rdmdevice.h"

namespace widget {
enum class Amf {
	START_CODE = 0x7E,	///< Start of message delimiter
	END_CODE = 0xE7		///< End of message delimiter
};

enum class SendState {
	ALWAYS = 0,				///< The widget will always send (default)
	ON_DATA_CHANGE_ONLY = 1	///< Requests the Widget to send a DMX packet to the host only when the DMX values change on the input port
};

enum class Mode {
	DMX_RDM = 0,	///< Both DMX (FIRMWARE_NORMAL_DMX)and RDM (FIRMWARE_RDM) firmware enabled.
	DMX = 1,		///< DMX (FIRMWARE_NORMAL_DMX) firmware enabled
	RDM = 2,		///< RDM (FIRMWARE_RDM) firmware enabled.
	RDM_SNIFFER = 3	///< RDM Sniffer firmware enabled.
};
}  // namespace widget

struct TRdmStatistics {
	uint32_t nDiscoveryPackets;
	uint32_t nDiscoveryResponsePackets;
	uint32_t nGetRequests;
	uint32_t nSetRequests;
};

class Widget: public Dmx, public RDMDevice {
public:
	Widget();

	void Init() {
		RDMDevice::Init();
	}

	widget::SendState GetReceiveDmxOnChange() const {
		return m_tReceiveDmxOnChange;
	}

	widget::Mode GetMode() const {
		return m_tMode;
	}

	void SetMode(widget::Mode mode)  {
		m_tMode = mode;
	}

	uint32_t GetReceivedDmxPacketPeriodMillis() const {
		return m_nReceivedDmxPacketPeriodMillis;
	}

	void SetReceivedDmxPacketPeriodMillis(uint32_t period) {
		m_nReceivedDmxPacketPeriodMillis = period;
	}

	uint32_t GetReceivedDmxPacketCount() const {
		return m_nReceivedDmxPacketCount;
	}

	const struct TRdmStatistics *RdmStatisticsGet() const {
		return &m_RdmStatistics;
	}

	void SnifferFillTransmitBuffer();

	void Run() {
		ReceiveDataFromHost();
		ReceivedDmxPacket();
		ReceivedDmxChangeOfStatePacket();
		ReceivedRdmPacket();
		RdmTimeout();
		SnifferRdm();
		SnifferDmx();
	}

	static Widget* Get() {
		return s_pThis;
	}

private:
	// Labels
	void GetParamsReply();
	void SetParams();
	void GetNameReply();
	void SendDmxPacketRequestOutputOnly(uint16_t nDataLength);
	void SendRdmPacketRequest(uint16_t nDataLength);
	void ReceiveDmxOnChange();
	void GetSnReply();
	void SendRdmDiscoveryRequest(uint16_t nDataLength);
	void GetManufacturerReply();
	void RdmTimeOutMessage();
	// Run
	void ReceiveDataFromHost();
	void ReceivedDmxPacket();
	void ReceivedDmxChangeOfStatePacket();
	void ReceivedRdmPacket();
	void RdmTimeout();
	void SnifferRdm();
	void SnifferDmx();
	// USB
	void SendMessage(uint8_t nLabel, const uint8_t *pData, uint32_t nLength);
	void SendHeader(uint8_t nLabel, uint32_t nLength);
	void SendData(const uint8_t *pData, uint32_t nLength);
	void SendFooter();
	//
	void UsbSendPackage(const uint8_t *pData, uint16_t nStart, uint16_t nDdataLength);
	bool UsbCanSend();

private:
#define WIDGET_DATA_BUFFER_SIZE		600
	uint8_t m_aData[WIDGET_DATA_BUFFER_SIZE];	///< Message between widget and the USB host
	widget::Mode m_tMode { widget::Mode::DMX_RDM };
	widget::SendState m_tReceiveDmxOnChange { widget::SendState::ALWAYS };
	uint32_t m_nReceivedDmxPacketPeriodMillis { 0 };
	uint32_t m_nReceivedDmxPacketStartMillis { 0 };
	uint32_t m_nSendRdmPacketStartMillis { 0 };
	bool m_isRdmDiscoveryRunning { false };
	uint32_t m_nReceivedDmxPacketCount { 0 };
	TRdmStatistics m_RdmStatistics;

	static Widget *s_pThis;
};

#endif /* WIDGET_H_ */
