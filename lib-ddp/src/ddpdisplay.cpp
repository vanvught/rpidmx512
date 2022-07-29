/**
 * @file ddpdisplay.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <ddpdisplaypixelconfiguration.h>
#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ddpdisplay.h"

#include "ws28xxmulti.h"

#include "dmx.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

using namespace ddp;

namespace json {
static constexpr char START[] = "{\"status\":{\"update\":\"change\",\"state\":\"up\"}}";
static constexpr char DISCOVER_REPLY[] = "{\"status\":{\"man\":\"www.orangepi-dmx.org\",\"mod\":\"Pixel\",\"ver\":\"1.0\"}}";
static constexpr char CONFIG_REPLY[] = "{\"config\":{\"ip\":\"%d.%d.%d.%d\",\"nm\":\"%d.%d.%d.%d\",\"gw\":\"%d.%d.%d.%d\",\"ports\":["
		"{\"port\":\"0\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"1\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"2\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"3\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"4\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"5\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"6\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
		"{\"port\":\"7\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"}"
		"]}}";

namespace size {
static constexpr auto START = sizeof(json::START) -1;
static constexpr auto DISCOVER_REPLY = sizeof(json::DISCOVER_REPLY) -1;
}  // namespace size
}  // namespace json

ddpdisplay::FrameBuffer DdpDisplay::s_FrameBuffer;
uint32_t DdpDisplay::s_nOffsetCompare[ddpdisplay::configuration::MAX_PORTS];

DdpDisplay::DdpDisplay(DdpDisplayPixelConfiguration& ddpPixelConfiguration) {
	ddpPixelConfiguration.Validate(m_nLedsPerPixel);
	ddpPixelConfiguration.Dump();

	m_pWS28xxMulti = new WS28xxMulti(ddpPixelConfiguration);
	assert(m_pWS28xxMulti != nullptr);

	m_pWS28xxMulti->Blackout();

	uint32_t nSum = 0;

	for (uint8_t nPixelPortIndex = 0; nPixelPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPixelPortIndex++) {
		auto *pPortData = &s_FrameBuffer.pixelPortdata[nPixelPortIndex];
		pPortData->length = static_cast<uint16_t>(ddpPixelConfiguration.GetCountPort(nPixelPortIndex) * m_nLedsPerPixel);
		nSum = nSum + pPortData->length;
		s_nOffsetCompare[nPixelPortIndex] = nSum;
	}

	for (uint8_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
		auto *pPortData = &s_FrameBuffer.dmxPortdata[nDmxPortIndex];
		pPortData->length = 512;
		nSum = nSum + pPortData->length;
		s_nOffsetCompare[nDmxPortIndex] = nSum;
	}
}

DdpDisplay::~DdpDisplay() {
	DEBUG_ENTRY

	Stop();

	delete m_pWS28xxMulti;

	DEBUG_EXIT
}

void DdpDisplay::Start() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

	memset(&m_Packet.header, 0, HEADER_LEN);
	m_Packet.header.flags1 = flags1::VER1 | flags1::REPLY;
	m_Packet.header.id = id::STATUS;
	m_Packet.header.len[1] = json::size::START;
	memcpy(m_Packet.data, json::START, json::size::START);

	Network::Get()->SendTo(m_nHandle, &m_Packet, HEADER_LEN + json::size::START, Network::Get()->GetIp() | ~(Network::Get()->GetNetmask()), udp::PORT);

	debug_dump(&m_Packet, HEADER_LEN + json::size::START);
	DEBUG_PUTS(json::START);

	for (uint8_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
		m_Dmx.SetPortDirection(nDmxPortIndex, dmx::PortDirection::OUTP, true);
	}
}

void DdpDisplay::Stop() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->End(udp::PORT);

	m_pWS28xxMulti->Blackout();

	for (uint8_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
		m_Dmx.SetPortDirection(nDmxPortIndex, dmx::PortDirection::OUTP, false);
	}

	DEBUG_EXIT
}

void DdpDisplay::HandleQuery() {
	DEBUG_ENTRY

	if ((m_Packet.header.id & id::STATUS) == id::STATUS) {
		DEBUG_PUTS("id::STATUS");

		m_Packet.header.flags1 = flags1::VER1 | flags1::REPLY | flags1::PUSH;
		m_Packet.header.len[0] = 0;
		m_Packet.header.len[1] = json::size::DISCOVER_REPLY;
		memcpy(m_Packet.data, json::DISCOVER_REPLY, json::size::DISCOVER_REPLY);

		Network::Get()->SendTo(m_nHandle, &m_Packet, HEADER_LEN + json::size::DISCOVER_REPLY, m_nFromIp, udp::PORT);

		debug_dump(&m_Packet, HEADER_LEN + json::size::DISCOVER_REPLY);
	}

	if ((m_Packet.header.id & id::STATUS) == id::CONFIG) {
		DEBUG_PUTS("id::CONFIG");

		const auto nLength = snprintf(reinterpret_cast<char *>(m_Packet.data), sizeof(m_Packet.data),
				json::CONFIG_REPLY,
				IP2STR(Network::Get()->GetIp()), IP2STR(Network::Get()->GetNetmask()), IP2STR(Network::Get()->GetGatewayIp()),
				s_FrameBuffer.pixelPortdata[0].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[1].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[2].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[3].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[4].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[5].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[6].length / m_nLedsPerPixel,
				s_FrameBuffer.pixelPortdata[7].length / m_nLedsPerPixel);

		m_Packet.header.flags1 = flags1::VER1 | flags1::REPLY | flags1::PUSH;
		m_Packet.header.len[0] = static_cast<uint8_t>(nLength >> 8);
		m_Packet.header.len[1] = static_cast<uint8_t>(nLength);

		Network::Get()->SendTo(m_nHandle, &m_Packet, static_cast<uint16_t>(HEADER_LEN + static_cast<uint16_t>(nLength)), m_nFromIp, udp::PORT);

		debug_dump(&m_Packet, static_cast<uint16_t>(HEADER_LEN + static_cast<uint16_t>(nLength)));
	}

	DEBUG_EXIT
}

void DdpDisplay::HandleData() {
	uint32_t i = 0;
	auto nOffset = static_cast<uint32_t>((m_Packet.header.offset[0] << 24) | (m_Packet.header.offset[1] << 16) |
			 	 	 	     (m_Packet.header.offset[2] << 8) | m_Packet.header.offset[3]);
	auto const nLength = static_cast<uint16_t>((m_Packet.header.len[0] << 8) | m_Packet.header.len[1]);
	const auto *receiveBuffer = m_Packet.data;

	DEBUG_PRINTF("nOffset=%u, nLength=%u", nOffset, nLength);

	if (nOffset < s_FrameBuffer.pixelPortdata[0].length) {
		for (; (nOffset < s_FrameBuffer.pixelPortdata[0].length) && (i < nLength); nOffset++) {
			s_FrameBuffer.pixelPortdata[0].data[nOffset] = receiveBuffer[i++];
		}
	}

	DEBUG_PRINTF("nOffset=%u, i=%u", nOffset, i);

	for (uint32_t nPortIndex = 1; (nPortIndex < ddpdisplay::configuration::MAX_PORTS) && (i < nLength); nPortIndex++) {
		if (nOffset < s_nOffsetCompare[nPortIndex]) {
			DEBUG_PRINTF("nPortIndex=%d, nOffset=%d, nOffsetCompare[%d]=%d, nOffsetCompare[%d - 1]=%d, nOffset - nOffsetCompare[nPort - 1]=%d", nPortIndex, nOffset, nPortIndex, s_nOffsetCompare[nPortIndex], nPortIndex, s_nOffsetCompare[nPortIndex - 1], nOffset - s_nOffsetCompare[nPortIndex - 1]);

			for (; (nOffset - s_nOffsetCompare[nPortIndex - 1] < s_FrameBuffer.pixelPortdata[nPortIndex].length) && (i < nLength); nOffset++) {
				s_FrameBuffer.pixelPortdata[nPortIndex].data[nOffset - s_nOffsetCompare[nPortIndex - 1]] = receiveBuffer[i++];
			}
		}

		DEBUG_PRINTF(" nOffset=%u, i=%u", nOffset, i);
	}

	if ((m_Packet.header.flags1 & flags1::PUSH) == flags1::PUSH) {
		while (m_pWS28xxMulti->IsUpdating()) {
			// wait for completion
		}

		uint32_t nPortIndex = 0;

		for (nPortIndex = 0; nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPortIndex++) {
			uint32_t nPixelIndex = 0;
			const auto *pPortData = &s_FrameBuffer.pixelPortdata[nPortIndex];

			if (m_nLedsPerPixel == 3) {
				for (uint32_t i = 0; i < pPortData->length; i = i + 3) {
					const auto nRed = pPortData->data[i];
					const auto nGreen = pPortData->data[i + 1];
					const auto nBlue = pPortData->data[i + 2];
					m_pWS28xxMulti->SetPixel(nPortIndex, nPixelIndex++, nRed, nGreen, nBlue);
				}
			} else {
				assert(m_nLedsPerPixel == 4);
				for (uint32_t i = 0; i < pPortData->length; i = i + 4) {
					const auto nRed = pPortData->data[i];
					const auto nGreen = pPortData->data[i + 1];
					const auto nBlue = pPortData->data[i + 2];
					const auto nWhite = pPortData->data[i + 3];
					m_pWS28xxMulti->SetPixel(nPortIndex, nPixelIndex++, nRed, nGreen, nBlue, nWhite);
				}
			}
		}

		m_pWS28xxMulti->Update();

		for (uint8_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
			const auto *pPortData = &s_FrameBuffer.dmxPortdata[nDmxPortIndex];
			m_Dmx.SetPortSendDataWithoutSC(nDmxPortIndex, pPortData->data, pPortData->length);
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

	debug_dump(&m_Packet, HEADER_LEN);

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
	m_pWS28xxMulti->Print();

	for (uint32_t nPixelPortIndex = 0; nPixelPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPixelPortIndex++) {
		printf(" Port [%u]=%u\n", 1 + nPixelPortIndex, s_FrameBuffer.pixelPortdata[nPixelPortIndex].length);
	}

	puts("DMX");
	for (uint32_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
		printf(" Port [%u]=%u\n", 1 + nDmxPortIndex, s_FrameBuffer.dmxPortdata[nDmxPortIndex].length);
	}
}
