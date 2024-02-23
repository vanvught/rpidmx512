/**
 * @file pp.cpp
 *
 */
/*
 *  Universal Discovery Protocol
 *  A UDP protocol for finding Etherdream/Heroic Robotics lighting devices
 *
 *  (c) 2012 Jas Strong and Jacob Potter
 *  <jasmine@electronpusher.org> <jacobdp@gmail.com>
 *
 *	PixelPusherBase/PixelPusherExt split created by Henner Zeller 2016
 *
 *	pusher command stuff added by Christopher Schardt 2017
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "pp.h"

#include "network.h"
#include "hardware.h"

#include "lightsetdata.h"

#include "debug.h"

#if !defined(CONFIG_PP_16BITSTUFF)
static constexpr uint8_t COMMAND_MAGIC[16] = { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50 };
#endif

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

PixelPusher *PixelPusher::s_pThis;

PixelPusher::PixelPusher(): m_nMillis(Hardware::Get()->Millis()) {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(&m_DiscoveryPacket, 0, sizeof(struct pp::DiscoveryPacket));

	Network::Get()->MacAddressCopyTo(m_DiscoveryPacket.header.mac_address);
	m_DiscoveryPacket.header.device_type = static_cast<uint8_t>(pp::DeviceType::PIXELPUSHER);
	m_DiscoveryPacket.header.protocol_version = 1;
	m_DiscoveryPacket.header.vendor_id = 3;
	// m_DiscoveryPacket.header.hw_revision = ;
	m_DiscoveryPacket.header.sw_revision = pp::version::MIN;
	m_DiscoveryPacket.header.link_speed = 10000000;
	//
	m_DiscoveryPacket.pixelpusher.base.update_period = 1000;
	m_DiscoveryPacket.pixelpusher.base.power_total = 1;
	m_DiscoveryPacket.pixelpusher.base.max_strips_per_packet = 1; //TODO This can be m_nActivePorts ?
	m_DiscoveryPacket.pixelpusher.base.my_port = pp::UDP_PORT_DATA;
	//
	m_DiscoveryPacket.pixelpusher.ext.segments = 1;

	DEBUG_EXIT
}

void PixelPusher::Start() {
	DEBUG_ENTRY
	assert(m_pLightSet != nullptr);

	m_nHandleDiscovery = Network::Get()->Begin(pp::UDP_PORT_DISCOVERY);
	assert(m_nHandleDiscovery != -1);

	m_nHandleData = Network::Get()->Begin(pp::UDP_PORT_DATA);
	assert(m_nHandleData != -1);

#if !defined(CONFIG_PP_16BITSTUFF)
	m_DiscoveryPacket.pixelpusher.base.strips_attached = static_cast<uint8_t>(m_nActivePorts);
#else
	m_DiscoveryPacket.pixelpusher.base.strips_attached = 1;
#endif
	m_DiscoveryPacket.pixelpusher.base.pixels_per_strip = static_cast<uint16_t>(m_nCount);

	m_DiscoveryPacket.pixelpusher.ext.strip_count_16 = static_cast<uint16_t>(m_nActivePorts);

#if !defined(CONFIG_PP_16BITSTUFF)
	m_DiscoveryPacket.pixelpusher.ext.pusher_flags = 0;
#else
	static const uint32_t nPusherFlags = (m_hasGlobalBrightness ? static_cast<uint32_t>(pp::PusherFlags::GLOBAL_BRIGHTNESS) : 0) | static_cast<uint32_t>(pp::PusherFlags::DYNAMICS) | static_cast<uint32_t>(pp::PusherFlags::_16BITSTUFF);
	m_DiscoveryPacket.pixelpusher.ext.pusher_flags = nPusherFlags;
#endif

	m_nStripDataLength = 1U +  m_nCount * pp::configuration::CHANNELS_PER_PIXEL;

	DEBUG_EXIT
}

void PixelPusher::Stop() {
	DEBUG_ENTRY

	m_nHandleData = Network::Get()->End(pp::UDP_PORT_DATA);
	m_nHandleData = -1;

	Network::Get()->End(pp::UDP_PORT_DISCOVERY);
	m_nHandleDiscovery = -1;

	DEBUG_EXIT
}

void PixelPusher::Run() {
	uint16_t nRemotePort;
	uint32_t nRemoteIP;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleData, const_cast<const void **>(reinterpret_cast<void **>(&m_pDataPacket)), &nRemoteIP, &nRemotePort);

	if (__builtin_expect((m_nBytesReceived < 4), 1)) {
		const auto nMillis = Hardware::Get()->Millis();
		if (__builtin_expect((nMillis - m_nMillis < 1000), 1)) {
			return;
		}
		m_nMillis = nMillis;
		_pcast32 src;
		src.u32 = Network::Get()->GetIp();
		memcpy(m_DiscoveryPacket.header.ip_address, src.u8, 4);
		Network::Get()->SendTo(m_nHandleDiscovery, reinterpret_cast<const void *>(&m_DiscoveryPacket), sizeof(struct pp::DiscoveryPacket),  static_cast<uint32_t>(~0), pp::UDP_PORT_DISCOVERY);
		return;
	}

	auto *pData = m_pDataPacket;

	uint32_t nSequenceNumber;
	memcpy(&nSequenceNumber, pData, 4);
	m_nBytesReceived -= 4;
	pData += 4;

#if !defined(CONFIG_PP_16BITSTUFF)
	if (m_nBytesReceived >= sizeof(COMMAND_MAGIC) && memcmp(pData, COMMAND_MAGIC, sizeof(COMMAND_MAGIC)) == 0) {
		HandlePusherCommand(pData + sizeof(COMMAND_MAGIC), m_nBytesReceived - sizeof(COMMAND_MAGIC));
		return;
	}

	if (m_nBytesReceived % m_nStripDataLength != 0) {
		DEBUG_PRINTF("Expecting multiple of {1 + (RGB)*%u} = %u but got %u bytes (leftover: %u)",
				m_nCount, m_nStripDataLength, m_nBytesReceived, m_nBytesReceived % m_nStripDataLength);
		return;
	}

	const auto nReceivedStrips = m_nBytesReceived / m_nStripDataLength;

	for (uint32_t i = 0; i < nReceivedStrips; i++) {
		const auto nPortIndexStart = pData[0] * m_nUniverses;
		uint32_t nPortIndex;
		for (nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + m_nUniverses) && (m_nBytesReceived > 0); nPortIndex++) {
			const auto nLightSetLength = std::min(std::min(m_nBytesReceived, pp::configuration::UNIVERSE_MAX_LENGTH), m_nStripDataLength - 1);

//			DEBUG_PRINTF("i=%u, nPortIndex=%u, m_nBytesReceived=%u, nLightSetLength=%u", i, nPortIndex, m_nBytesReceived, nLightSetLength);

			lightset::Data::SetSourceA(nPortIndex, &pData[1], nLightSetLength);

			m_nBytesReceived-=nLightSetLength;
			pData+=nLightSetLength;
		}

		pData+=m_nStripDataLength;

//		DEBUG_PRINTF("nPortIndex=%u, m_nPortIndexLast=%u", nPortIndex, m_nPortIndexLast);

		if (nPortIndex == m_nPortIndexLast) {
//			DEBUG_PUTS("Output");
			for (uint32_t nLightSetPortIndex = 0; nLightSetPortIndex < m_nPortIndexLast; nLightSetPortIndex++) {
				lightset::Data::Output(m_pLightSet, nLightSetPortIndex);
				lightset::Data::ClearLength(nLightSetPortIndex);
			}
		}
	}
#else
#endif
}

void PixelPusher::HandlePusherCommand([[maybe_unused]] const uint8_t *pBuffer, [[maybe_unused]] uint32_t nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("pBuffer=%p, nSize=%u", reinterpret_cast<const void *>(pBuffer), nSize);
#if !defined(CONFIG_PP_16BITSTUFF)
#else
#endif
	DEBUG_EXIT
}

#include <cstdio>

void PixelPusher::Print() {
	puts("PixelPusher");
	printf(" Count             : %u\n", m_nCount);
	printf(" Channels per pixel: %u\n", pp::configuration::CHANNELS_PER_PIXEL);
	printf(" Active ports      : %u\n", m_nActivePorts);
	DEBUG_PRINTF("m_nUniverses=%u", m_nUniverses);
	DEBUG_PRINTF("m_nPortIndexLast=%u", m_nPortIndexLast);
}
