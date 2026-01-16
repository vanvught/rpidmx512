/**
 * @file widget.h
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
 *
 */
/* Copyright (C) 2015-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "usb.h"

namespace widget
{
enum class Amf
{
    kStartCode = 0x7E, ///< Start of message delimiter
    kEndCode = 0xE7    ///< End of message delimiter
};

enum class SendState
{
    kAlways = 0,          ///< The widget will always send (default)
    kOnDataChangeOnly = 1 ///< Requests the Widget to send a DMX packet to the host only when the DMX values change on the input port
};

enum class Mode
{
    kDmxRdm = 0,    ///< Both DMX (FIRMWARE_NORMAL_DMX)and RDM (FIRMWARE_RDM) firmware enabled.
    kDmx = 1,       ///< DMX (FIRMWARE_NORMAL_DMX) firmware enabled
    kRdm = 2,       ///< RDM (FIRMWARE_RDM) firmware enabled.
    kRdmSniffer = 3 ///< RDM Sniffer firmware enabled.
};
} // namespace widget

struct TRdmStatistics
{
    uint32_t discovery_packets;
    uint32_t discovery_response_packets;
    uint32_t get_requests;
    uint32_t set_requests;
};

class Widget : public Dmx
{
   public:
    Widget();

    void Init() { RdmDevice::Get().Init(); }

    widget::SendState GetReceiveDmxOnChange() const { return send_state_; }

    widget::Mode GetMode() const { return mode_; }

    void SetMode(widget::Mode mode) { mode_ = mode; }

    uint32_t GetReceivedDmxPacketPeriodMillis() const { return received_dmx_packet_period_millis_; }

    void SetReceivedDmxPacketPeriodMillis(uint32_t period) { received_dmx_packet_period_millis_ = period; }

    uint32_t GetReceivedDmxPacketCount() const { return received_dmx_packet_count_; }

    const struct TRdmStatistics* RdmStatisticsGet() const { return &rdm_statistics_; }

    void SnifferFillTransmitBuffer();

    void Run()
    {
        ReceiveDataFromHost();
        ReceivedDmxPacket();
        ReceivedDmxChangeOfStatePacket();
        ReceivedRdmPacket();
        RdmTimeout();
        SnifferRdm();
        SnifferDmx();
    }

    static Widget* Get() { return s_this; }

   private:
    // Labels
    void GetParamsReply();
    void SetParams();
    void GetNameReply();
    void SendDmxPacketRequestOutputOnly(uint16_t data_length);
    void SendRdmPacketRequest(uint16_t data_length);
    void ReceiveDmxOnChange();
    void GetSnReply();
    void SendRdmDiscoveryRequest(uint16_t data_length);
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
    void SendHeader(uint8_t label, uint32_t length)
    {
        usb_send_byte(static_cast<uint8_t>(widget::Amf::kStartCode));
        usb_send_byte(label);
        usb_send_byte(static_cast<uint8_t>(length & 0x00FF));
        usb_send_byte(static_cast<uint8_t>(length >> 8));
    }

    void SendMessage(uint8_t label, const uint8_t* data, uint32_t length)
    {
        SendHeader(label, length);
        SendData(data, length);
        SendFooter();
    }

    void SendData(const uint8_t* data, uint32_t length)
    {
        for (uint32_t i = 0; i < length; i++)
        {
            usb_send_byte(data[i]);
        }
    }

    void SendFooter() { usb_send_byte(static_cast<uint8_t>(widget::Amf::kEndCode)); }
    //
    void UsbSendPackage(const uint8_t* data, uint16_t start, uint16_t data_length);
    bool UsbCanSend();

   private:
    static constexpr uint32_t kWidgetDataBufferSize = 600;
    uint8_t data_[kWidgetDataBufferSize]; ///< Message between widget and the USB host
    widget::Mode mode_{widget::Mode::kDmxRdm};
    widget::SendState send_state_{widget::SendState::kAlways};
    uint32_t received_dmx_packet_period_millis_{0};
    uint32_t received_dmx_packet_start_millis_{0};
    uint32_t send_rdm_packet_start_millis_{0};
    bool is_rdm_discovery_running_{false};
    uint32_t received_dmx_packet_count_{0};
    TRdmStatistics rdm_statistics_;

    inline static Widget* s_this;
};

#endif // WIDGET_H_
