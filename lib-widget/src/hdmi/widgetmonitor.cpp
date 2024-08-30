/**
 * @file widgetmonitor.cpp
 *
 */
/* Copyright (C) 2015-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hardware.h"

static constexpr auto DISPLAY_LEVEL = 1;

/* RDM START CODE (Slot 0)                                                                                                     */
#define	E120_SC_RDM		0xCC

uint32_t WidgetMonitor::s_nWidgetReceivedDmxPacketCountPrevious { 0 };

uint32_t WidgetMonitor::s_nUpdatesPerSecondeMin { UINT32_MAX };
uint32_t WidgetMonitor::s_nUpdatesPerSecondeMax { 0 };
uint32_t WidgetMonitor::s_nSlotsInPacketMin { UINT32_MAX };
uint32_t WidgetMonitor::s_nSlotsInPacketMax { 0 };
uint32_t WidgetMonitor::s_nSlotToSlotMin { UINT32_MAX };
uint32_t WidgetMonitor::s_nSlotToSlotMax { 0 };
uint32_t WidgetMonitor::s_nBreakToBreakMin { UINT32_MAX };
uint32_t WidgetMonitor::s_nBreakToBreakMax { 0 };

using namespace widget;
using namespace widgetmonitor;
using namespace dmxsingle;
using namespace dmx;

void WidgetMonitor::Uptime(uint8_t nLine) {
	auto nUptime = Hardware::Get()->GetUpTime();
	auto ltime = time(nullptr);
	auto *pLocalTime = localtime(&ltime);

	console_save_cursor();
	console_set_cursor(0, nLine);

	const uint32_t nDays = nUptime / (24 * 3600);
	nUptime -= nDays * (24 * 3600);
	const uint32_t nHours = nUptime / 3600;
	nUptime -= nHours * 3600;
	const uint32_t nMinutes = nUptime / 60;
	const uint32_t nSeconds = nUptime - nMinutes * 60;

	printf("Local time %.2d:%.2d:%.2d, uptime %d days, %02d:%02d:%02d",
			pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec, nDays,
			nHours, nMinutes, nSeconds);

	console_restore_cursor();
}

void WidgetMonitor::DmxData(const uint8_t * pDmxData, const int nLine) {
	uint32_t slots;

	if (PortDirection::INP == Dmx::Get()->GetPortDirection(0)) {
		const struct Data *dmx_statistics = (struct Data *)pDmxData;
		slots = dmx_statistics->Statistics.nSlotsInPacket + 1;
	} else {
		slots = Dmx::Get()->GetSendDataLength();
	}

	console_set_cursor(0, nLine);

	console_puts("01-16 : ");

	if (slots > 33) {
		slots = 33;
	}

	uint32_t i;

	for (i = 1; i < slots; i++) {
		uint8_t d = pDmxData[i];
		if (d == 0) {
			console_puts(" 0");
		} else {
			console_puthex_fg_bg(d, (uint16_t)(d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), (uint16_t)RGB(d,d,d));
		}
		if (i == 16) {
			console_puts("\n17-32 : ");
		} else {
			console_putc((int) ' ');
		}

	}

	for (; i < 33; i++) {
		console_puts("--");
		if (i == 16) {
			console_puts("\n17-32 : ");
		} else {
			console_putc(' ');
		}
	}
}

void WidgetMonitor::Sniffer() {
	const volatile auto *total_statistics = Dmx::Get()->GetTotalStatistics();
	const auto total_packets = total_statistics->nDmxPackets + total_statistics->nRdmPackets;
	const auto *dmx_data = Dmx::Get()->GetDmxCurrentData(0);
	const auto *dmx_statistics = (struct Data *)dmx_data;
	const auto dmx_updates_per_seconde = Dmx::Get()->GetUpdatesPerSecond(0);
	const volatile auto *rdm_statistics = Widget::Get()->RdmStatisticsGet();

	DmxData(dmx_data, MonitorLine::DMX_DATA);

	WidgetMonitor::Line(MonitorLine::PACKETS, "Packets : %u, DMX %u, RDM %u\n\n", total_packets, total_statistics->nDmxPackets, total_statistics->nRdmPackets);

	printf("Discovery          : %u\n", rdm_statistics->nDiscoveryPackets);
	printf("Discovery response : %u\n", rdm_statistics->nDiscoveryResponsePackets);
	printf("GET Requests       : %u\n", rdm_statistics->nGetRequests);
	printf("SET Requests       : %u\n", rdm_statistics->nSetRequests);

	if (dmx_updates_per_seconde != 0) {
		s_nUpdatesPerSecondeMin = std::min(dmx_updates_per_seconde, s_nUpdatesPerSecondeMin);
		s_nUpdatesPerSecondeMax = std::max(dmx_updates_per_seconde, s_nUpdatesPerSecondeMax);
		printf("\nDMX updates/sec     %3d     %3d / %d\n\n", (int)dmx_updates_per_seconde, (int)s_nUpdatesPerSecondeMin , (int)s_nUpdatesPerSecondeMax);
	} else {
		console_puts("\nDMX updates/sec --     \n\n");
	}

	if (dmx_updates_per_seconde != 0) {
		s_nSlotsInPacketMin = std::min(dmx_statistics->Statistics.nSlotsInPacket, s_nSlotsInPacketMin);
		s_nSlotsInPacketMax = std::max(dmx_statistics->Statistics.nSlotsInPacket, s_nSlotsInPacketMax);
		s_nSlotToSlotMin = std::min(dmx_statistics->Statistics.nSlotToSlot, s_nSlotToSlotMin);
		s_nSlotToSlotMax = std::max(dmx_statistics->Statistics.nSlotToSlot, s_nSlotToSlotMax);
		s_nBreakToBreakMin = std::min(dmx_statistics->Statistics.nBreakToBreak, s_nBreakToBreakMin);
		s_nBreakToBreakMax = std::max(dmx_statistics->Statistics.nBreakToBreak, s_nBreakToBreakMax);

		printf("Slots in packet     %3d     %3d / %d\n", (int)dmx_statistics->Statistics.nSlotsInPacket, (int)s_nSlotsInPacketMin, (int)s_nSlotsInPacketMax);
		printf("Slot to slot        %3d     %3d / %d\n", (int)dmx_statistics->Statistics.nSlotToSlot, (int)s_nSlotToSlotMin, (int)s_nSlotToSlotMax);
		printf("Break to break   %6d  %6d / %d\n", (int)dmx_statistics->Statistics.nBreakToBreak, (int)s_nBreakToBreakMin, (int)s_nBreakToBreakMax);
	} else {
		console_puts("Slots in packet --     \n");
		console_puts("Slot to slot    --     \n");
		console_puts("Break to break  --     \n");
	}
}

void WidgetMonitor::Update() {
	const auto tMode = Widget::Get()->GetMode();

	if (tMode != Mode::RDM_SNIFFER && DISPLAY_LEVEL == 0) {
		return;
	}

	struct TWidgetConfiguration widget_params;
	WidgetConfiguration::Get(&widget_params);

	if (DISPLAY_LEVEL > 1) {
		WidgetMonitor::Uptime(MonitorLine::TIME);
	}

	console_clear_line(MonitorLine::INFO);

	if (tMode == Mode::RDM_SNIFFER) {
		Sniffer();
	} else {
		console_clear_line(MonitorLine::WIDGET_PARMS);

		printf("Firmware %d.%d BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d(%d)",
				widget_params.nFirmwareMsb, widget_params.nFirmwareLsb,
				widget_params.nBreakTime, Dmx::Get()->GetDmxBreakTime(),
				widget_params.nMabTime, Dmx::Get()->GetDmxMabTime(),
				widget_params.nRefreshRate, (1000000 / Dmx::Get()->GetDmxPeriodTime()));

		console_clear_line(MonitorLine::PORT_DIRECTION);

		if (PortDirection::INP == Dmx::Get()->GetPortDirection(0)) {
			const auto receive_dmx_on_change =  Widget::Get()->GetReceiveDmxOnChange();

			if (receive_dmx_on_change == SendState::ALWAYS) {
				const auto throttle = Widget::Get()->GetReceivedDmxPacketPeriodMillis();
				const auto widget_received_dmx_packet_count = Widget::Get()->GetReceivedDmxPacketCount();

				console_puts("Input [SEND_ALWAYS]");

				if (throttle != (uint32_t) 0) {
					printf(", Throttle %d", (int) (1E6 / throttle));
				}

				WidgetMonitor::Line(MonitorLine::STATS, "DMX packets per second to host : %d", widget_received_dmx_packet_count - s_nWidgetReceivedDmxPacketCountPrevious);
				s_nWidgetReceivedDmxPacketCountPrevious = widget_received_dmx_packet_count;
			} else {
				console_puts("Input [SEND_ON_DATA_CHANGE_ONLY]");
				console_clear_line(MonitorLine::STATS);
			}
		} else {
			console_puts("Output");
			console_clear_line(MonitorLine::STATS);
		}

		const auto *dmx_data = Dmx::Get()->GetDmxCurrentData(0);
		DmxData(dmx_data, MonitorLine::DMX_DATA);
	}
}

void WidgetMonitor::RdmData(int nLine, uint16_t nDataLength, const uint8_t *pData, bool isSent) {
	auto *p = (uint8_t *) pData;
	bool is_rdm_command = (*p == E120_SC_RDM);

	WidgetMonitor::Line(nLine, "RDM [%s], l:%d\n", isSent ? "Sent" : "Received", (int) nDataLength);

	if (DISPLAY_LEVEL == 0) {
		return;
	}

	uint32_t l;

	for (l = 0; l < std::min(nDataLength, static_cast<uint16_t>(28)); l++) {
		console_puthex(*p++);
		console_putc((int) ' ');
	}

	while (l++ < 26) {
		console_puts("   ");
	}

	if (is_rdm_command) {
		if (nDataLength >= 24) {
			auto *cmd = (struct TRdmMessage *) (pData);
			WidgetMonitor::Line(nLine + 2, "tn:%d, cc:%.2x, pid:%d, l:%d", (int)cmd->transaction_number, (unsigned int)cmd->command_class, (int)((cmd->param_id[0] << 8) + cmd->param_id[1]), (int) cmd->param_data_length);
			console_clear_line(nLine + 4);
			console_clear_line(nLine + 3);
			if (cmd->param_data_length != 0) {
				auto i = std::min(cmd->param_data_length, static_cast<uint8_t>(24));
				uint8_t j;

				p = &cmd->param_data[0];

				for (j = 0 ; j < i; j++) {
					console_puthex(*p++);
					console_putc((int) ' ');
				}

				p = &cmd->param_data[0];
				console_set_cursor(0, nLine + 4);

				for (j = 0 ; j < i; j++) {
					if (isprint((int) *p)) {
						console_putc((int) *p);
					} else {
						console_putc((int) '.');
					}
					(void) console_puts("  ");
					p++;
				}
			}
		}
	} else {
		console_clear_line(nLine + 2);
		console_clear_line(nLine + 3);
		console_clear_line(nLine + 4);
	}
}
