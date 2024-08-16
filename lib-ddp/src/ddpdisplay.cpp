/**
 * @file ddpdisplay.h
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "ddpdisplay.h"

#include "lightsetdata.h"
#include "lightset.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

using namespace ddp;

namespace json {
static constexpr char START[] = "{\"status\":{\"update\":\"change\",\"state\":\"up\"}}";
static constexpr char DISCOVER_REPLY[] = "{\"status\":{\"man\":\"%s\",\"mod\":\"Pixel\",\"ver\":\"1.0\",\"mac\":\"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\"}}";
static constexpr char CONFIG_REPLY[] = "{\"config\":{\"ip\":\"%d.%d.%d.%d\",\"nm\":\"%d.%d.%d.%d\",\"gw\":\"%d.%d.%d.%d\",\"ports\":["
		"{\"port\":\"0\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"1\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"2\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"3\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
#if CONFIG_PIXELDMX_MAX_PORTS > 2
		"{\"port\":\"4\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"5\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"6\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"7\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"8\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"9\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"}"
#endif
#if CONFIG_PIXELDMX_MAX_PORTS == 16
		","
		"{\"port\":\"10\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"11\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"12\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"13\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"14\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"15\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"16\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"17\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"}"

#endif
		"]}}";

namespace size {
static constexpr auto START = sizeof(json::START) - 1U;
}  // namespace size
}  // namespace json

uint32_t DdpDisplay::s_nLightsetPortLength[ddpdisplay::lightset::MAX_PORTS];
uint32_t DdpDisplay::s_nOffsetCompare[ddpdisplay::configuration::MAX_PORTS];
DdpDisplay *DdpDisplay::s_pThis;

DdpDisplay::DdpDisplay() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	Network::Get()->MacAddressCopyTo(m_macAddress);

	DEBUG_EXIT
}

DdpDisplay::~DdpDisplay() {
	DEBUG_ENTRY

	Stop();

	DEBUG_EXIT
}

void DdpDisplay::CalculateOffsets() {
	uint32_t nSum = 0;

	for (uint32_t nPixelPortIndex = 0; nPixelPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPixelPortIndex++) {
		nSum = nSum + m_nStripDataLength;
		s_nOffsetCompare[nPixelPortIndex] = nSum;
	}

/*
  error: comparison of unsigned expression < 0 is always false [-Werror=type-limits]
  for (uint32_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
 */

#if __GNUC__ < 10
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wtype-limits"	// FIXME ignored "-Wtype-limits"
#endif

	for (uint32_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
		nSum = nSum + lightset::dmx::UNIVERSE_SIZE;
		const auto nIndexOffset = nDmxPortIndex + ddpdisplay::configuration::pixel::MAX_PORTS;
		s_nOffsetCompare[nIndexOffset] = nSum;
	}

#if __GNUC__ < 10
# pragma GCC diagnostic pop
#endif

}

void DdpDisplay::Start() {
	DEBUG_ENTRY
	assert(m_pLightSet != nullptr);

	m_nHandle = Network::Get()->Begin(ddp::UDP_PORT);
	assert(m_nHandle != -1);

	memset(&m_Packet.header, 0, HEADER_LEN);
	m_Packet.header.flags1 = flags1::VER1 | flags1::REPLY;
	m_Packet.header.id = id::STATUS;
	m_Packet.header.len[1] = json::size::START;
	memcpy(m_Packet.data, json::START, json::size::START);

	Network::Get()->SendTo(m_nHandle, &m_Packet, HEADER_LEN + json::size::START, Network::Get()->GetIp() | ~(Network::Get()->GetNetmask()), ddp::UDP_PORT);

	debug_dump(&m_Packet, HEADER_LEN + json::size::START);

	CalculateOffsets();
	DEBUG_EXIT
}

void DdpDisplay::Stop() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->End(ddp::UDP_PORT);
	m_nHandle = -1;

	DEBUG_EXIT
}

void DdpDisplay::HandleQuery() {
	DEBUG_ENTRY

	if ((m_Packet.header.id & id::STATUS) == id::STATUS) {
		DEBUG_PUTS("id::STATUS");

		const auto nLength = snprintf(reinterpret_cast<char *>(m_Packet.data), sizeof(m_Packet.data),
				json::DISCOVER_REPLY, Hardware::Get()->GetWebsiteUrl(), MAC2STR(m_macAddress) );

		m_Packet.header.flags1 = flags1::VER1 | flags1::REPLY | flags1::PUSH;
		m_Packet.header.len[0] = static_cast<uint8_t>(nLength >> 8);
		m_Packet.header.len[1] = static_cast<uint8_t>(nLength);

		Network::Get()->SendTo(m_nHandle, &m_Packet, (HEADER_LEN + static_cast<uint16_t>(nLength)), Network::Get()->GetIp() | ~(Network::Get()->GetNetmask()), ddp::UDP_PORT);
	}

	if ((m_Packet.header.id & id::STATUS) == id::CONFIG) {
		DEBUG_PUTS("id::CONFIG");

		const auto nLength = snprintf(reinterpret_cast<char *>(m_Packet.data), sizeof(m_Packet.data),
				json::CONFIG_REPLY,
				IP2STR(Network::Get()->GetIp()), IP2STR(Network::Get()->GetNetmask()), IP2STR(Network::Get()->GetGatewayIp()),
				m_nActivePorts > 0 ? m_nCount : 0,
				m_nActivePorts > 1 ? m_nCount : 0,
#if CONFIG_PIXELDMX_MAX_PORTS > 2
				m_nActivePorts > 2 ? m_nCount : 0,
				m_nActivePorts > 3 ? m_nCount : 0,
				m_nActivePorts > 4 ? m_nCount : 0,
				m_nActivePorts > 5 ? m_nCount : 0,
				m_nActivePorts > 6 ? m_nCount : 0,
				m_nActivePorts > 7 ? m_nCount : 0,
#endif
#if CONFIG_PIXELDMX_MAX_PORTS == 16
				m_nActivePorts >  8 ? m_nCount : 0,
				m_nActivePorts >  9 ? m_nCount : 0,
				m_nActivePorts > 10 ? m_nCount : 0,
				m_nActivePorts > 11 ? m_nCount : 0,
				m_nActivePorts > 12 ? m_nCount : 0,
				m_nActivePorts > 13 ? m_nCount : 0,
				m_nActivePorts > 14 ? m_nCount : 0,
				m_nActivePorts > 15 ? m_nCount : 0,
#endif
				ddpdisplay::configuration::dmx::MAX_PORTS == 0 ? 0 : lightset::dmx::UNIVERSE_SIZE,
				ddpdisplay::configuration::dmx::MAX_PORTS == 0 ? 0 : lightset::dmx::UNIVERSE_SIZE
				);

		m_Packet.header.flags1 = flags1::VER1 | flags1::REPLY | flags1::PUSH;
		m_Packet.header.len[0] = static_cast<uint8_t>(nLength >> 8);
		m_Packet.header.len[1] = static_cast<uint8_t>(nLength);

		Network::Get()->SendTo(m_nHandle, &m_Packet, HEADER_LEN + nLength, m_nFromIp, ddp::UDP_PORT);

		debug_dump(&m_Packet, HEADER_LEN + nLength);
	}

	DEBUG_EXIT
}

void DdpDisplay::HandleData() {
	auto nOffset = static_cast<uint32_t>(
			  (m_Packet.header.offset[0] << 24)
			| (m_Packet.header.offset[1] << 16)
			| (m_Packet.header.offset[2] << 8)
			|  m_Packet.header.offset[3]);

	auto nLength = ((static_cast<uint32_t>(m_Packet.header.len[0]) << 8) | m_Packet.header.len[1]);
	const auto *receiveBuffer = m_Packet.data;

//	DEBUG_PRINTF("nOffset=%u, nLength=%u, s_nOffsetCompare[0]=%u", nOffset, nLength, s_nOffsetCompare[0]);

	uint32_t nLightSetPortIndex = 0;
	uint32_t nReceiverBufferIndex = 0;

	for (uint32_t nPortIndex = 0; (nPortIndex < m_nActivePorts) && (nLength != 0); nPortIndex++) {
		nLightSetPortIndex = nPortIndex * 4;

		const auto nLightSetPortIndexEnd = nLightSetPortIndex + 4;

//		DEBUG_PRINTF("nOffset=%u, nLength=%u, s_nOffsetCompare[%u]=%u", nOffset, nLength, nPortIndex, s_nOffsetCompare[nPortIndex]);

		while ((nOffset < s_nOffsetCompare[nPortIndex]) && (nLightSetPortIndex < nLightSetPortIndexEnd)) {
			const auto nLightSetLength = std::min(std::min(nLength, m_nLightSetDataMaxLength), m_nStripDataLength);

//			DEBUG_PRINTF("==> nOffset=%u, nLength=%u, nLightSetLength=%u, nLightSetPortIndex=%u", nOffset, nLength, nLightSetLength, nLightSetPortIndex);

			lightset::Data::SetSourceA(nLightSetPortIndex, &receiveBuffer[nReceiverBufferIndex], nLightSetLength);
			s_nLightsetPortLength[nLightSetPortIndex] = nLightSetLength;

			nReceiverBufferIndex += nLightSetLength;
			nOffset += nLightSetLength;
			nLength -= nLightSetLength;
			nLightSetPortIndex++;

//			DEBUG_PRINTF("nOffset=%u, nLength=%u, nLightSetLength=%u, nLightSetPortIndex=%u", nOffset, nLength, nLightSetLength, nLightSetPortIndex);
		}
	}

	/*
	 * 2x DMX ports
	 */

	nLightSetPortIndex = ddpdisplay::lightset::MAX_PORTS - ddpdisplay::configuration::dmx::MAX_PORTS;

	DEBUG_PRINTF("nLightSetPortIndex=%u", nLightSetPortIndex);

	for (uint32_t nPortIndex = ddpdisplay::configuration::pixel::MAX_PORTS; (nPortIndex < ddpdisplay::configuration::MAX_PORTS) && (nLength != 0); nPortIndex++) {
		if (nOffset < s_nOffsetCompare[nPortIndex]) {
			const auto nLightSetLength = std::min(nLength,lightset::dmx::UNIVERSE_SIZE);

//			DEBUG_PRINTF("==> nPortIndex=%u, nOffset=%u, nLength=%u, nLightSetLength=%u, nLightSetPortIndex=%u", nPortIndex, nOffset, nLength, nLightSetLength, nLightSetPortIndex);

			lightset::Data::SetSourceA(nLightSetPortIndex, &receiveBuffer[nReceiverBufferIndex], nLightSetLength);
			s_nLightsetPortLength[nLightSetPortIndex] = nLightSetLength;

			nReceiverBufferIndex += nLightSetLength;
			nOffset += nLightSetLength;
			nLength -= nLightSetLength;
			nLightSetPortIndex++;

//			DEBUG_PRINTF("nPortIndex=%u, nOffset=%u, nLength=%u, nLightSetLength=%u, nLightSetPortIndex=%u", nPortIndex, nOffset, nLength, nLightSetLength, nLightSetPortIndex);
		}
	}

	if ((m_Packet.header.flags1 & flags1::PUSH) == flags1::PUSH) {
		for (uint32_t nLightSetPortIndex = 0; nLightSetPortIndex < ddpdisplay::lightset::MAX_PORTS; nLightSetPortIndex++) {
			lightset::Data::Output(m_pLightSet, nLightSetPortIndex);
			lightset::Data::ClearLength(nLightSetPortIndex);
		}
	}
}

void DdpDisplay::Run() {
	uint16_t nFromPort;

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Packet, sizeof(m_Packet), &m_nFromIp, &nFromPort);

	if (__builtin_expect((nBytesReceived < HEADER_LEN), 1)) {
		return;
	}

	if (m_nFromIp == Network::Get()->GetIp()) {
		DEBUG_PUTS("Own message");
		return;
	}

	if ((m_Packet.header.flags1 & flags1::VER_MASK) != flags1::VER1) {
		DEBUG_PUTS("Invalid version");
		return;
	}

	if (m_Packet.header.id == id::DISPLAY) {
		HandleData();
		return;
	}

	if ((m_Packet.header.flags1 & flags1::QUERY) == flags1::QUERY) {
		HandleQuery();
		return;
	}
}

void DdpDisplay::Print() {
	puts("DDP Display");
	printf(" Count             : %u\n", m_nCount);
	printf(" Channels per pixel: %u\n", GetChannelsPerPixel());
	printf(" Active ports      : %u\n", m_nActivePorts);
}
