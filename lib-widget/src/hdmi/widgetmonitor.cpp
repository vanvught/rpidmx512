/**
 * @file widgetmonitor.cpp
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
#include <cctype>
#include <cstdio>
#include <algorithm>

#include "widgetmonitor.h"
#include "widget.h"
#include "widgetconfiguration.h"

#include "console.h"
#include "dmx.h"
#include "rdm.h"
#include "hal.h"

static constexpr auto DISPLAY_LEVEL = 1;

/* RDM START CODE (Slot 0)                                                                                                     */
#define E120_SC_RDM 0xCC

using namespace widget;
using namespace widgetmonitor;
using namespace dmxsingle;
using namespace dmx;

void WidgetMonitor::Uptime(uint8_t line)
{
    auto nUptime = hal::Uptime();
    auto ltime = time(nullptr);
    auto* pLocalTime = localtime(&ltime);

    console::SaveCursor();
    console::SetCursor(0, line);

    const uint32_t nDays = nUptime / (24 * 3600);
    nUptime -= nDays * (24 * 3600);
    const uint32_t hours = nUptime / 3600;
    nUptime -= hours * 3600;
    const uint32_t minutes = nUptime / 60;
    const uint32_t seconds = nUptime - minutes * 60;

    printf("Local time %.2d:%.2d:%.2d, uptime %d days, %02d:%02d:%02d", pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec, nDays, hours, minutes,
           seconds);

    RestoreCursor();
}

void WidgetMonitor::DmxData(const uint8_t* pDmxData, int line)
{
    uint32_t slots;

    if (PortDirection::kInput == Dmx::Get()->GetPortDirection(0))
    {
        const struct Data* dmx_statistics = (struct Data*)pDmxData;
        slots = dmx_statistics->Statistics.nSlotsInPacket + 1;
    }
    else
    {
        slots = Dmx::Get()->GetSendDataLength();
    }

    console::SetCursor(0, line);

    Puts("01-16 : ");

    if (slots > 33)
    {
        slots = 33;
    }

    uint32_t i;

    for (i = 1; i < slots; i++)
    {
        uint8_t d = pDmxData[i];
        if (d == 0)
        {
            Puts(" 0");
        }
        else
        {
            PuthexFgBg(d, (uint16_t)(d > 92 ? CONSOLE_BLACK : console::Colours::kConsoleWhite), (uint16_t)RGB(d, d, d));
        }
        if (i == 16)
        {
            Puts("\n17-32 : ");
        }
        else
        {
            Putc((int)' ');
        }
    }

    for (; i < 33; i++)
    {
        Puts("--");
        if (i == 16)
        {
            Puts("\n17-32 : ");
        }
        else
        {
            Putc(' ');
        }
    }
}

void WidgetMonitor::Sniffer()
{
    const volatile auto* total_statistics = Dmx::Get()->GetTotalStatistics();
    const auto total_packets = total_statistics->nDmxPackets + total_statistics->nRdmPackets;
    const auto* dmx_data = Dmx::Get()->GetDmxCurrentData(0);
    const auto* dmx_statistics = (struct Data*)dmx_data;
    const auto dmx_updates_per_seconde = Dmx::Get()->GetUpdatesPerSecond(0);
    const volatile auto* rdm_statistics = Widget::Get()->RdmStatisticsGet();

    DmxData(dmx_data, MonitorLine::kDmxData);

    WidgetMonitor::Line(MonitorLine::kPackets, "Packets : %u, DMX %u, RDM %u\n\n", total_packets, total_statistics->nDmxPackets, total_statistics->nRdmPackets);

    printf("Discovery          : %u\n", rdm_statistics->discovery_packets);
    printf("Discovery response : %u\n", rdm_statistics->discovery_response_packets);
    printf("GET Requests       : %u\n", rdm_statistics->get_requests);
    printf("SET Requests       : %u\n", rdm_statistics->set_requests);

    if (dmx_updates_per_seconde != 0)
    {
        s_nUpdatesPerSecondeMin = std::min(dmx_updates_per_seconde, s_nUpdatesPerSecondeMin);
        s_nUpdatesPerSecondeMax = std::max(dmx_updates_per_seconde, s_nUpdatesPerSecondeMax);
        printf("\nDMX updates/sec     %3d     %3d / %d\n\n", (int)dmx_updates_per_seconde, (int)s_nUpdatesPerSecondeMin, (int)s_nUpdatesPerSecondeMax);
    }
    else
    {
        Puts("\nDMX updates/sec --     \n\n");
    }

    if (dmx_updates_per_seconde != 0)
    {
        s_nSlotsInPacketMin = std::min(dmx_statistics->Statistics.nSlotsInPacket, s_nSlotsInPacketMin);
        s_nSlotsInPacketMax = std::max(dmx_statistics->Statistics.nSlotsInPacket, s_nSlotsInPacketMax);
        s_nSlotToSlotMin = std::min(dmx_statistics->Statistics.nSlotToSlot, s_nSlotToSlotMin);
        s_nSlotToSlotMax = std::max(dmx_statistics->Statistics.nSlotToSlot, s_nSlotToSlotMax);
        s_nBreakToBreakMin = std::min(dmx_statistics->Statistics.nBreakToBreak, s_nBreakToBreakMin);
        s_nBreakToBreakMax = std::max(dmx_statistics->Statistics.nBreakToBreak, s_nBreakToBreakMax);

        printf("Slots in packet     %3d     %3d / %d\n", (int)dmx_statistics->Statistics.nSlotsInPacket, (int)s_nSlotsInPacketMin, (int)s_nSlotsInPacketMax);
        printf("Slot to slot        %3d     %3d / %d\n", (int)dmx_statistics->Statistics.nSlotToSlot, (int)s_nSlotToSlotMin, (int)s_nSlotToSlotMax);
        printf("Break to break   %6d  %6d / %d\n", (int)dmx_statistics->Statistics.nBreakToBreak, (int)s_nBreakToBreakMin, (int)s_nBreakToBreakMax);
    }
    else
    {
        Puts("Slots in packet --     \n");
        Puts("Slot to slot    --     \n");
        Puts("Break to break  --     \n");
    }
}

void WidgetMonitor::Update()
{
    const auto tMode = Widget::Get()->GetMode();

    if (tMode != Mode::kRdmSniffer && DISPLAY_LEVEL == 0)
    {
        return;
    }

    struct TWidgetConfiguration widget_params;
    WidgetConfiguration::Get(&widget_params);

    if (DISPLAY_LEVEL > 1)
    {
        WidgetMonitor::Uptime(MonitorLine::kTime);
    }

    ClearLine(MonitorLine::kInfo);

    if (tMode == Mode::kRdmSniffer)
    {
        Sniffer();
    }
    else
    {
        ClearLine(MonitorLine::kWidgetParms);

        printf("Firmware %d.%d BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d(%d)", widget_params.firmware_msb, widget_params.firmware_lsb,
               widget_params.break_time, Dmx::Get()->GetDmxBreakTime(), widget_params.mab_time, Dmx::Get()->GetDmxMabTime(), widget_params.refresh_rate,
               (1000000 / Dmx::Get()->GetDmxPeriodTime()));

        ClearLine(MonitorLine::kPortDirection);

        if (PortDirection::kInput == Dmx::Get()->GetPortDirection(0))
        {
            const auto receive_dmx_on_change = Widget::Get()->GetReceiveDmxOnChange();

            if (receive_dmx_on_change == SendState::kAlways)
            {
                const auto throttle = Widget::Get()->GetReceivedDmxPacketPeriodMillis();
                const auto widget_received_dmx_packet_count = Widget::Get()->GetReceivedDmxPacketCount();

                Puts("Input [SEND_ALWAYS]");

                if (throttle != (uint32_t)0)
                {
                    printf(", Throttle %d", (int)(1E6 / throttle));
                }

                WidgetMonitor::Line(MonitorLine::kStats, "DMX packets per second to host : %d",
                                    widget_received_dmx_packet_count - s_nWidgetReceivedDmxPacketCountPrevious);
                s_nWidgetReceivedDmxPacketCountPrevious = widget_received_dmx_packet_count;
            }
            else
            {
                Puts("Input [SEND_ON_DATA_CHANGE_ONLY]");
                ClearLine(MonitorLine::kStats);
            }
        }
        else
        {
            Puts("Output");
            ClearLine(MonitorLine::kStats);
        }

        const auto* dmx_data = Dmx::Get()->GetDmxCurrentData(0);
        DmxData(dmx_data, MonitorLine::kDmxData);
    }
}

void WidgetMonitor::RdmData(int line, uint16_t nDataLength, const uint8_t* pData, bool isSent)
{
    auto* p = (uint8_t*)pData;
    bool is_rdm_command = (*p == E120_SC_RDM);

    WidgetMonitor::Line(line, "RDM [%s], l:%d\n", isSent ? "Sent" : "Received", (int)nDataLength);

    if (DISPLAY_LEVEL == 0)
    {
        return;
    }

    uint32_t l;

    for (l = 0; l < std::min(nDataLength, static_cast<uint16_t>(28)); l++)
    {
        ConsolePuthex(*p++);
        Putc((int)' ');
    }

    while (l++ < 26)
    {
        Puts("   ");
    }

    if (is_rdm_command)
    {
        if (nDataLength >= 24)
        {
            auto* cmd = (struct TRdmMessage*)(pData);
            WidgetMonitor::Line(line + 2, "tn:%d, cc:%.2x, pid:%d, l:%d", (int)cmd->transaction_number, (unsigned int)cmd->command_class,
                                (int)((cmd->param_id[0] << 8) + cmd->param_id[1]), (int)cmd->param_data_length);
            ClearLine(line + 4);
            ClearLine(line + 3);
            if (cmd->param_data_length != 0)
            {
                auto i = std::min(cmd->param_data_length, static_cast<uint8_t>(24));
                uint8_t j;

                p = &cmd->param_data[0];

                for (j = 0; j < i; j++)
                {
                    ConsolePuthex(*p++);
                    Putc((int)' ');
                }

                p = &cmd->param_data[0];
                console::SetCursor(0, line + 4);

                for (j = 0; j < i; j++)
                {
                    if (isprint((int)*p))
                    {
                        Putc((int)*p);
                    }
                    else
                    {
                        Putc((int)'.');
                    }
                    (void)Puts("  ");
                    p++;
                }
            }
        }
    }
    else
    {
        ClearLine(line + 2);
        ClearLine(line + 3);
        ClearLine(line + 4);
    }
}
