/**
 * @file widget.cpp
 *
 * @brief DMX USB Pro Widget API Specification 1.44
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
 *
 */
/* Copyright (C) 2015-2025 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "widget.h"
#include "widgetconfiguration.h"
#if !defined(NO_HDMI_OUTPUT)
#include "widgetmonitor.h"
#endif
#include "hal_millis.h"
#include "dmx.h"
#include "rdm.h"
#include "rdmdevice.h"
#include "rdm_e120.h"
#include "usb.h"

enum
{
    GET_WIDGET_PARAMS = 3,                   ///< Get Widget Parameters Request
    GET_WIDGET_PARAMS_REPLY = 3,             ///< Get Widget Parameters Reply
    SET_WIDGET_PARAMS = 4,                   ///< Set Widget Parameters Request
    RECEIVED_DMX_PACKET = 5,                 ///< Received DMX Packet
    OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST = 6, ///< Output Only Send DMX Packet Request
    SEND_RDM_PACKET_REQUEST = 7,             ///< Send RDM Packet Request
    RECEIVE_DMX_ON_CHANGE = 8,               ///< Receive DMX on Change
    RECEIVED_DMX_COS_TYPE = 9,               ///< Received DMX Change Of State Packet
    GET_WIDGET_SN_REQUEST = 10,              ///< Get Widget Serial Number Request
    GET_WIDGET_SN_REPLY = 10,                ///< Get Widget Serial Number Reply
    SEND_RDM_DISCOVERY_REQUEST = 11,         ///< Send RDM Discovery Request
    RDM_TIMEOUT = 12,                        ///< https://github.com/OpenLightingProject/ola/blob/master/plugins/usbpro/EnttecUsbProWidget.cpp#L353
    MANUFACTURER_LABEL = 77,                 ///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
    GET_WIDGET_NAME_LABEL = 78               ///< https://wiki.openlighting.org/index.php/USB_Protocol_Extensions
};

Widget::Widget()
{
    assert(s_this == nullptr);
    s_this = this;

    usb_init();

    SetOutputStyle(0, dmx::OutputStyle::kConstant);
    SetPortDirection(0, dmx::PortDirection::kInput, false);
}

/*
 * Widget LABELs
 */

/**
 *
 * Get Widget Parameters Reply (Label=3 \ref GET_WIDGET_PARAMS_REPLY)
 * The Widget sends this message to the PC in response to the Get Widget Parameters request.
 */
void Widget::GetParamsReply()
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "GET_WIDGET_PARAMS_REPLY");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    TWidgetConfiguration widget_configuration;
    WidgetConfiguration::Get(&widget_configuration);
    SendMessage(GET_WIDGET_PARAMS_REPLY, reinterpret_cast<uint8_t*>(&widget_configuration), sizeof(struct TWidgetConfiguration));
}

/**
 *
 * Set Widget Parameters Request (Label=4 SET_WIDGET_PARAMS)
 * This message sets the Widget configuration. The Widget configuration is preserved when the Widget loses power.
 *
 */
void Widget::SetParams()
{
    TWidgetConfiguration widget_configuration;

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "SET_WIDGET_PARAMS");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    SetPortDirection(0, dmx::PortDirection::kInput, false);

    widget_configuration.break_time = data_[2];
    widget_configuration.mab_time = data_[3];
    widget_configuration.refresh_rate = data_[4];
    WidgetConfiguration::Store(&widget_configuration);

    SetPortDirection(0, dmx::PortDirection::kInput, true);

    received_dmx_packet_start_millis_ = hal::Millis();
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
void Widget::ReceivedDmxPacket()
{
    if (mode_ == widget::Mode::kRdmSniffer)
    {
        return;
    }

    if (is_rdm_discovery_running_ || (dmx::PortDirection::kInput != GetPortDirection(0)) || (widget::SendState::kOnDataChangeOnly == send_state_))
    {
        return;
    }

    const auto* dmx_data_available = GetDmxAvailable(0);

    if (dmx_data_available == nullptr)
    {
        return;
    }

    const auto kMillis = hal::Millis();

    if (kMillis - received_dmx_packet_start_millis_ < received_dmx_packet_period_millis_)
    {
        return;
    }

    received_dmx_packet_start_millis_ = kMillis;
    received_dmx_packet_count_++;

    const auto* dmx_statistics = reinterpret_cast<const struct Data*>(dmx_data_available);
    const auto kLength = dmx_statistics->Statistics.nSlotsInPacket + 1;

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kLabel, "RECEIVED_DMX_PACKET");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Send DMX data to HOST, %d", kLength);
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    SendHeader(RECEIVED_DMX_PACKET, static_cast<uint16_t>(kLength + 1));
    usb_send_byte(0); // DMX Receive status
    SendData(dmx_data_available, static_cast<uint16_t>(kLength));
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
void Widget::ReceivedRdmPacket()
{
    if ((mode_ == widget::Mode::kDmx) || (mode_ == widget::Mode::kRdmSniffer) || (send_state_ == widget::SendState::kOnDataChangeOnly))
    {
        return;
    }

    const auto* rdm_data = Rdm::Receive(0);

    if (rdm_data == nullptr)
    {
        return;
    }

    uint8_t message_length = 0;

    if (rdm_data[0] == E120_SC_RDM)
    {
        const auto* p = reinterpret_cast<const struct TRdmMessage*>(rdm_data);
        const auto kCommandClass = p->command_class;
        message_length = static_cast<uint8_t>(p->message_length + 2);

#if !defined(NO_HDMI_OUTPUT)
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Send RDM data to HOST, l:%d", message_length);
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, "RECEIVED_RDM_PACKET SC:0xCC");
#endif

        SendHeader(RECEIVED_DMX_PACKET, static_cast<uint32_t>(1 + message_length));
        usb_send_byte(0); // RDM Receive status
        SendData(rdm_data, message_length);
        SendFooter();

        const auto kParamId = static_cast<uint16_t>((p->param_id[0] << 8) + p->param_id[1]);

        if ((kCommandClass == E120_DISCOVERY_COMMAND_RESPONSE) && (kParamId != E120_DISC_MUTE))
        {
            RdmTimeOutMessage();
        }
        else
        {
            send_rdm_packet_start_millis_ = 0;
        }
    }
    else if (rdm_data[0] == 0xFE)
    {
        message_length = 24;

#if !defined(NO_HDMI_OUTPUT)
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Send RDM data to HOST, l:%d", message_length);
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, "RECEIVED_RDM_PACKET SC:0xFE");
#endif

        SendHeader(RECEIVED_DMX_PACKET, static_cast<uint32_t>(1 + message_length));
        usb_send_byte(0); // RDM Receive status
        SendData(rdm_data, message_length);
        SendFooter();

        RdmTimeOutMessage();
    }

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::RdmData(widgetmonitor::MonitorLine::kRdmData, message_length, rdm_data, false);
#endif
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
void Widget::SendDmxPacketRequestOutputOnly(uint16_t data_length)
{
    if (send_rdm_packet_start_millis_ != 0)
    {
        return;
    }

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "OUTPUT_ONLY_SEND_DMX_PACKET_REQUEST");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    Dmx::SetPortDirection(0, dmx::PortDirection::kOutput, false);
    Dmx::SetSendData<dmx::SendStyle::kDirect>(0, data_, data_length);
    Dmx::SetPortDirection(0, dmx::PortDirection::kOutput, true);
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
void Widget::SendRdmPacketRequest(uint16_t data_length)
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "SEND_RDM_PACKET_REQUEST");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    const auto* data = reinterpret_cast<const struct TRdmMessage*>(data_);

    is_rdm_discovery_running_ = (data->command_class == E120_DISCOVERY_COMMAND);

    Rdm::SendRaw(0, data_, data_length);

    send_rdm_packet_start_millis_ = hal::Millis();

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::RdmData(widgetmonitor::MonitorLine::kRdmData, data_length, data_, true);
#endif
}

/**
 *
 * This function is called from Run
 *
 */
void Widget::RdmTimeout()
{
    if (mode_ == widget::Mode::kRdmSniffer)
    {
        return;
    }

    if (send_rdm_packet_start_millis_ == 0)
    {
        return;
    }

    if (hal::Millis() - send_rdm_packet_start_millis_ < 1000U)
    { // 1 second
        return;
    }

    RdmTimeOutMessage(); // Send message to host Label=12 RDM_TIMEOUT
    send_rdm_packet_start_millis_ = 0;
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
void Widget::ReceiveDmxOnChange()
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "RECEIVE_DMX_ON_CHANGE");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    send_state_ = static_cast<widget::SendState>(data_[0]);

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, false);
    Dmx::ClearData(0);
    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, true);

    received_dmx_packet_start_millis_ = hal::Millis();
}

/**
 *
 * Received DMX Change Of State Packet (Label = 9 \ref RECEIVED_DMX_COS_TYPE)
 *
 * The Widget sends one or more instances of this message to the PC unsolicited, whenever the
 * Widget receives a changed DMX packet from the DMX port, and the Receive DMX on Change
 * mode (\ref receive_dmx_on_change) is 'Send on data change only' (\ref SEND_ON_DATA_CHANGE_ONLY).
 */
void Widget::ReceivedDmxChangeOfStatePacket()
{
    if (mode_ == widget::Mode::kRdmSniffer)
    {
        return;
    }

    if (is_rdm_discovery_running_ || (dmx::PortDirection::kInput != GetPortDirection(0)) || (widget::SendState::kAlways == send_state_))
    {
        return;
    }

    if (nullptr != Dmx::GetDmxChanged(0))
    {
#if !defined(NO_HDMI_OUTPUT)
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "RECEIVED_DMX_COS_TYPE");
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
        // TODO (a) widget_received_dmx_change_of_state_packet
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Sent changed DMX data to HOST");
#endif
    }
}

/**
 *
 * Get Widget Serial Number Reply (Label = 10 GET_WIDGET_PARAMS_REPLY)
 *
 */
void Widget::GetSnReply()
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "GET_WIDGET_PARAMS_REPLY");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, false);

    SendMessage(GET_WIDGET_SN_REPLY, RdmDevice::Get().GetSN(), DEVICE_SN_LENGTH);

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, true);

    received_dmx_packet_start_millis_ = hal::Millis();
}

/**
 *
 * Send RDM Discovery Request (Label=11 SEND_RDM_DISCOVERY_REQUEST)
 *
 * This message requests the Widget to send an RDM Discovery Request packet out of the Widget
 * DMX port, and then receive an RDM Discovery Response.
 */
void Widget::SendRdmDiscoveryRequest(uint16_t data_length)
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "SEND_RDM_DISCOVERY_REQUEST");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    Rdm::SendRaw(0, data_, data_length);

    is_rdm_discovery_running_ = true;
    send_rdm_packet_start_millis_ = hal::Millis();

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::RdmData(widgetmonitor::MonitorLine::kRdmData, data_length, data_, true);
#endif
}

/**
 *
 * See https://github.com/OpenLightingProject/ola/blob/master/plugins/usbpro/EnttecUsbProWidget.cpp#L353
 *
 * (Label=12 RDM_TIMEOUT)
 *
 */
void Widget::RdmTimeOutMessage()
{
    const auto kMessageLength = 0;

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Send RDM data to HOST, l:%d", kMessageLength);
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, "RDM_TIMEOUT");
#endif

    SendHeader(RDM_TIMEOUT, kMessageLength);
    SendFooter();

    is_rdm_discovery_running_ = false;
    send_rdm_packet_start_millis_ = 0;
}

/**
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Manufacturer.2C_Label_.3D_77.2C_no_data
 *
 * Get Widget Manufacturer Reply (Label = 77  MANUFACTURER_LABEL)
 */
void Widget::GetManufacturerReply()
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "MANUFACTURER_LABEL");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    struct rdm::DeviceInfoData manufacturer_name;
    RdmDevice::Get().GetManufacturerName(&manufacturer_name);

    struct rdm::DeviceInfoData manufacturer_id;
    RdmDevice::Get().GetManufacturerId(&manufacturer_id);

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, false);

    SendHeader(MANUFACTURER_LABEL, static_cast<uint32_t>(manufacturer_id.length + manufacturer_name.length));
    SendData(reinterpret_cast<uint8_t*>(manufacturer_id.data), manufacturer_id.length);
    SendData(reinterpret_cast<uint8_t*>(manufacturer_name.data), manufacturer_name.length);
    SendFooter();

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, true);

    received_dmx_packet_start_millis_ = hal::Millis();
}

/**
 *
 * https://wiki.openlighting.org/index.php/USB_Protocol_Extensions#Device_Name.2C_Label_.3D_78.2C_no_data
 *
 * Get Widget Name Reply (Label = 78 GET_WIDGET_NAME_LABEL)
 */
void Widget::GetNameReply()
{
#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "GET_WIDGET_NAME_LABEL");
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kStatus, nullptr);
#endif

    struct rdm::DeviceInfoData widget_label;
    RdmDevice::Get().GetLabel(&widget_label);

    TWidgetConfigurationData widget_type_id;
    WidgetConfiguration::GetTypeId(&widget_type_id);

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, false);

    SendHeader(GET_WIDGET_NAME_LABEL, static_cast<uint32_t>(widget_type_id.length + widget_label.length));
    SendData(widget_type_id.data, widget_type_id.length);
    SendData(reinterpret_cast<uint8_t*>(widget_label.data), widget_label.length);
    SendFooter();

    Dmx::SetPortDirection(0, dmx::PortDirection::kInput, true);

    received_dmx_packet_start_millis_ = hal::Millis();
}

/**
 *
 * Read bytes from host
 *
 * This function is called from Run
 */
void Widget::ReceiveDataFromHost()
{
    if (usb_read_is_byte_available())
    {
        const auto kByte = usb_read_byte();

        if (static_cast<uint8_t>(widget::Amf::kStartCode) == kByte)
        {
            const auto kLabel = usb_read_byte();
            const auto kLsb = usb_read_byte();
            const auto kMsb = usb_read_byte();
            const auto kDataLength = static_cast<uint16_t>((kMsb << 8) | kLsb);

            uint32_t i;

            for (i = 0; i < kDataLength; i++)
            {
                data_[i] = usb_read_byte();
            }

            while ((static_cast<uint8_t>(widget::Amf::kEndCode) != usb_read_byte()) && (i++ < (sizeof(data_) / sizeof(data_[0]))));

#if !defined(NO_HDMI_OUTPUT)
            WidgetMonitor::Line(widgetmonitor::MonitorLine::kLabel, "L:%d:%d(%d)", kLabel, kDataLength, i);
#endif

            switch (kLabel)
            {
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
                    SendDmxPacketRequestOutputOnly(kDataLength);
                    break;
                case RECEIVE_DMX_ON_CHANGE:
                    ReceiveDmxOnChange();
                    break;
                case SEND_RDM_PACKET_REQUEST:
                    SendRdmPacketRequest(kDataLength);
                    break;
                case SEND_RDM_DISCOVERY_REQUEST:
                    SendRdmDiscoveryRequest(kDataLength);
                    break;
                default:
                    break;
            }
        }
    }
}
