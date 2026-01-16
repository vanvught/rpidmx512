/**
 * @file widgetsniffer.cpp
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

#include <cstdint>

#include "widget.h"
#include "hal_micros.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "usb.h"
#if !defined(NO_HDMI_OUTPUT)
#include "widgetmonitor.h"
#endif

#define SNIFFER_PACKET 0x81     ///< Label
#define SNIFFER_PACKET_SIZE 200 ///< Packet size
#define CONTROL_MASK 0x00       ///< If the high bit is set, this is a data byte, otherwise it's a control byte
#define DATA_MASK 0x80          ///< If the high bit is set, this is a data byte, otherwise it's a control byte

void Widget::UsbSendPackage(const uint8_t* data, uint16_t start, uint16_t data_length)
{
    uint32_t i;

    if (data_length < (SNIFFER_PACKET_SIZE / 2))
    {
        SendHeader(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

        for (i = 0; i < data_length; i++)
        {
            usb_send_byte(DATA_MASK);
            usb_send_byte(data[i + start]);
        }

        for (i = data_length; i < SNIFFER_PACKET_SIZE / 2; i++)
        {
            usb_send_byte(CONTROL_MASK);
            usb_send_byte(0x02);
        }

        SendFooter();
    }
    else
    {
        SendHeader(SNIFFER_PACKET, SNIFFER_PACKET_SIZE);

        for (i = 0; i < SNIFFER_PACKET_SIZE / 2; i++)
        {
            usb_send_byte(DATA_MASK);
            usb_send_byte(data[i + start]);
        }

        SendFooter();

        UsbSendPackage(data, static_cast<uint16_t>(start + SNIFFER_PACKET_SIZE / 2), static_cast<uint16_t>(data_length - SNIFFER_PACKET_SIZE / 2));
    }
}

bool Widget::UsbCanSend()
{
    const auto kMicros = hal::Micros();

    while (!usb_can_write() && (hal::Micros() - kMicros < 1000))
    {
    }

    if (!usb_can_write())
    {
#if !defined(NO_HDMI_OUTPUT)
        WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "!Failed! Cannot send to host");
#endif
        return false;
    }

    return true;
}

/**
 * This function is called from Run
 */
void Widget::SnifferDmx()
{
    if ((GetMode() != widget::Mode::kRdmSniffer) || !UsbCanSend())
    {
        return;
    }

    const auto* dmx_data_changed = Dmx::GetDmxChanged(0);

    if (dmx_data_changed == nullptr)
    {
        return;
    }

    const auto* dmx_statistics = reinterpret_cast<const struct Data*>(dmx_data_changed);
    const auto kDataLength = dmx_statistics->Statistics.nSlotsInPacket + 1;

    if (!UsbCanSend())
    {
        return;
    }

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Send DMX data to HOST -> %d", kDataLength);
#endif
    UsbSendPackage(dmx_data_changed, 0, static_cast<uint16_t>(kDataLength));
}

/**
 * This function is called from Run
 */
void Widget::SnifferRdm()
{
    if ((GetMode() != widget::Mode::kRdmSniffer) || !UsbCanSend())
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
        message_length = static_cast<uint8_t>(p->message_length + 2);
        switch (p->command_class)
        {
            case E120_DISCOVERY_COMMAND:
                rdm_statistics_.discovery_packets++;
                break;
            case E120_DISCOVERY_COMMAND_RESPONSE:
                rdm_statistics_.discovery_response_packets++;
                break;
            case E120_GET_COMMAND:
                rdm_statistics_.get_requests++;
                break;
            case E120_SET_COMMAND:
                rdm_statistics_.set_requests++;
                break;
            default:
                break;
        }
    }
    else if (rdm_data[0] == 0xFE)
    {
        rdm_statistics_.discovery_response_packets++;
        message_length = 24;
    }

    if (!UsbCanSend())
    {
        return;
    }

#if !defined(NO_HDMI_OUTPUT)
    WidgetMonitor::Line(widgetmonitor::MonitorLine::kInfo, "Send RDM data to HOST");
#endif
    UsbSendPackage(rdm_data, 0, message_length);
}

void Widget::SnifferFillTransmitBuffer()
{
    if (!UsbCanSend())
    {
        return;
    }

    int i = 256;

    while (i--)
    {
        if (!UsbCanSend())
        {
            return;
        }
        usb_send_byte(0);
    }
}
