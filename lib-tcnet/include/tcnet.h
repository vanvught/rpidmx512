/**
 * @file tcnet.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef TCNET_H_
#define TCNET_H_

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <cassert>

#include "tcnetconst.h"
#include "tcnetpackets.h"
#include "tcnettimecode.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

namespace tcnet {
static constexpr char NODE_NAME_DEFAULT[] = "AvV";

enum class Layer {
	LAYER_1,
	LAYER_2,
	LAYER_3,
	LAYER_4,
	LAYER_A,
	LAYER_B,
	LAYER_M,
	LAYER_C,
	LAYER_UNDEFINED
};
}  // namespace tcnet

class TCNet {
public:
	TCNet() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		memset(&m_PacketOptIn, 0, sizeof(struct TTCNetPacketOptIn));

		// Fill the static fields for Opt-IN
		m_PacketOptIn.ManagementHeader.ProtocolVersionMajor = 3;
		m_PacketOptIn.ManagementHeader.ProtocolVersionMinor = 3;
		memcpy(m_PacketOptIn.ManagementHeader.Header, "TCN", 3);
		m_PacketOptIn.ManagementHeader.MessageType = TCNET_MESSAGE_TYPE_OPTIN;
		m_PacketOptIn.ManagementHeader.SEQ = 0;
		m_PacketOptIn.ManagementHeader.NodeType = TCNET_TYPE_SLAVE;
		m_PacketOptIn.ManagementHeader.NodeOptions = 0;
		m_PacketOptIn.NodeCount = 1;
		m_PacketOptIn.NodeListenerPort = TCNETUnicast::PORT;
		memcpy(&m_PacketOptIn.VendorName, "orangepi-dmx.org", TCNET_VENDOR_NAME_LENGTH);
		memcpy(&m_PacketOptIn.DeviceName, "LTC SMPTE Node  ", TCNET_DEVICE_NAME_LENGTH);
		m_PacketOptIn.DeviceMajorVersion = static_cast<uint8_t>(_TIME_STAMP_YEAR_ - 2000);
		m_PacketOptIn.DeviceMinorVersion = _TIME_STAMP_MONTH_;
		m_PacketOptIn.DeviceBugVersion = _TIME_STAMP_DAY_;

		SetNodeName(tcnet::NODE_NAME_DEFAULT);
		SetLayer(tcnet::Layer::LAYER_M);
		SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS);

		DEBUG_EXIT
	}

	void Start() {
		DEBUG_ENTRY

		m_aHandles[0] = Network::Get()->Begin(TCNetBroadcast::PORT_0);
		assert(m_aHandles[0] >= 0);

		m_aHandles[1] = Network::Get()->Begin(TCNetBroadcast::PORT_1);
		assert(m_aHandles[1] >= 0);

#if defined(USE_PORT_60002)
		m_aHandles[2] = Network::Get()->Begin(TCNetBroadcast::PORT_2);
		assert(m_aHandles[2] >= 0);
#endif

#if defined(USE_PORT_UNICAST)
		m_aHandles[3] = Network::Get()->Begin(TCNETUnicast::PORT);
		assert(m_aHandles[3] >= 0);
#endif
		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		TTCNetPacketOptOut OptOut;

		memcpy(&OptOut, &m_PacketOptIn, sizeof(struct TTCNetPacketOptOut));

		Network::Get()->SendTo(m_aHandles[0], &OptOut, sizeof(struct TTCNetPacketOptOut), Network::Get()->GetBroadcastIp(), TCNetBroadcast::PORT_0);

		DEBUG_EXIT
	}

	void Run() {
		uint16_t nForeignPort;

		auto nBytesReceived = Network::Get()->RecvFrom(m_aHandles[1], const_cast<const void**>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);

		if (nBytesReceived != 0) {
			HandlePort60001Incoming();
		}

		nBytesReceived = Network::Get()->RecvFrom(m_aHandles[0], const_cast<const void**>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);

		if (nBytesReceived != 0) {
			HandlePort60000Incoming();
		}

#if defined(USE_PORT_60002)
		nBytesReceived = Network::Get()->RecvFrom(m_aHandles[2], const_cast<const void**>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);

		if (nBytesReceived != 0) {
			HandlePort60002Incoming();
		}
#endif

#if defined(USE_PORT_UNICAST)
		nBytesReceived = Network::Get()->RecvFrom(m_aHandles[3], const_cast<const void**>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);

		if (nBytesReceived != 0) {
			HandlePortUnicastIncoming();
		}
#endif

		m_nCurrentMillis = Hardware::Get()->Millis();

		if (__builtin_expect(((m_nCurrentMillis - m_nPreviousMillis) >= 1000), 0)) {
			m_nPreviousMillis = m_nCurrentMillis;
			HandleOptInOutgoing();
		}
	}

	void Print() {
		puts("TCNet");
		printf(" Node : %.8s\n", m_PacketOptIn.ManagementHeader.NodeName);
		printf(" L%c", GetLayerName(m_Layer));
		if (m_bUseTimeCode) {
			puts(" TC");
		} else {
			printf(" T%u\n", TCNetConst::FPS[static_cast<uint32_t>(m_TimeCodeType)]);
		}

		printf("%u:%u:%u\n", static_cast<unsigned>(m_Layer), m_nLxTimeOffset, m_nLxTimeCodeOffset);
	}

	TTCNetNodeType GetNodeType() const {
		return static_cast<TTCNetNodeType>(m_PacketOptIn.ManagementHeader.NodeType);
	}

	void SetNodeName(const char *pNodeName) {
		strncpy(reinterpret_cast<char*>(m_PacketOptIn.ManagementHeader.NodeName), pNodeName, sizeof m_PacketOptIn.ManagementHeader.NodeName - 1);
		m_PacketOptIn.ManagementHeader.NodeName[sizeof m_PacketOptIn.ManagementHeader.NodeName - 1] = '\0';
	}

	const char *GetNodeName() {
		return reinterpret_cast<char*>(m_PacketOptIn.ManagementHeader.NodeName);
	}

	void SetLayer(const tcnet::Layer layer) {
		m_Layer = layer;
		m_nLxTimeOffset = offsetof(struct TTCNetPacketTime, L1Time) + (4 * static_cast<uint32_t>(layer));
		m_nLxTimeCodeOffset = offsetof(struct TTCNetPacketTime, L1TimeCode) + static_cast<uint32_t>(layer) * sizeof(struct TTCNetPacketTimeTimeCode);
	}

	tcnet::Layer GetLayer() const {
		return m_Layer;
	}

	void SetUseTimeCode(bool bUseTimeCode) {
		m_bUseTimeCode = bUseTimeCode;
	}
	bool GetUseTimeCode() const {
		return m_bUseTimeCode;
	}

	void SetTimeCodeType(const tcnet::TimeCodeType type) {
		switch (type) {
		case tcnet::TimeCodeType::TIMECODE_TYPE_FILM:
			m_fTypeDivider = 1000.0f / 24;
			break;
		case tcnet::TimeCodeType::TIMECODE_TYPE_EBU_25FPS:
			m_fTypeDivider = 1000.0f / 25;
			break;
		case tcnet::TimeCodeType::TIMECODE_TYPE_DF:
			m_fTypeDivider = 1000.0f / 29.97f;
			break;
		case tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS:
			m_fTypeDivider = 1000.0f / 30;
			break;
		default:
			return;
			break;
		}

		m_TimeCodeType = type;
	}

	tcnet::TimeCodeType GetTimeCodeType() const {
		return m_TimeCodeType;
	}

#if defined (TCNET_HAVE_TIMECODE)
	void SetArtTimeCodeCallbackFunction(TCNetTimeCodeCallbackFunctionPtr tcnetTimeCodeCallbackFunctionPtr) {
		m_TCNetTimeCodeCallbackFunctionPtr = tcnetTimeCodeCallbackFunctionPtr;
	}
#endif

public:
	static char GetLayerName(const tcnet::Layer layer) {
		switch (layer) {
		case tcnet::Layer::LAYER_1:
		case tcnet::Layer::LAYER_2:
		case tcnet::Layer::LAYER_3:
		case tcnet::Layer::LAYER_4:
			return static_cast<char>(static_cast<char>(layer) + '1');
			break;
		case tcnet::Layer::LAYER_A:
			return 'A';
			break;
		case tcnet::Layer::LAYER_B:
			return 'B';
			break;
		case tcnet::Layer::LAYER_M:
			return 'M';
			break;
		case tcnet::Layer::LAYER_C:
			return 'C';
			break;
		default:
			break;
		}

		return ' ';
	}

	static tcnet::Layer GetLayer(const char nChar) {
		switch (nChar | 0x20) {	// to lower case
		case '1':
		case '2':
		case '3':
		case '4':
			return static_cast<tcnet::Layer>(nChar - '1');
			break;
		case 'a':
			return tcnet::Layer::LAYER_A;
			break;
		case 'b':
			return tcnet::Layer::LAYER_B;
			break;
		case 'm':
			return tcnet::Layer::LAYER_M;
			break;
		case 'c':
			return tcnet::Layer::LAYER_C;
			break;
		default:
			break;
		}

		return tcnet::Layer::LAYER_UNDEFINED;
	}

	static TCNet *Get() {
		return s_pThis;
	}

private:
	void HandlePort60000Incoming() {
		const auto *pPacket = reinterpret_cast<struct TTCNetPacketManagementHeader *>(m_pReceiveBuffer);
		const auto messageType  = static_cast<TTCNetMessageType>(pPacket->MessageType);

		DEBUG_PRINTF("MessageType = %d", static_cast<int>(messageType));

		if (messageType == TCNET_MESSAGE_TYPE_OPTIN) {
#ifndef NDEBUG
			DumpOptIn();
#endif
			return;
		}
	}

	void HandlePort60001Incoming() {
#if defined (TCNET_HAVE_TIMECODE)
		const auto *pPacketTime = reinterpret_cast<struct TTCNetPacketTime *>(m_pReceiveBuffer);

		if (static_cast<TTCNetMessageType>(pPacketTime->ManagementHeader.MessageType) == TCNET_MESSAGE_TYPE_TIME) {
			tcnet::TimeCode timeCode __attribute__ ((aligned (4)));

			if (m_bUseTimeCode) {
				const auto *pTC =  reinterpret_cast<TTCNetPacketTimeTimeCode *>(m_pReceiveBuffer + m_nLxTimeCodeOffset);
				timeCode.nFrames = pTC->Frames;
				timeCode.nSeconds = pTC->Seconds;
				timeCode.nMinutes = pTC->Minutes;
				timeCode.nHours = pTC->Hours;

				auto nSMPTEMode = pTC->SMPTEMode;

				if (nSMPTEMode < 24) {
					nSMPTEMode = pPacketTime->SMPTEMode;
				}

				switch (nSMPTEMode) {
				case 24:
					timeCode.nType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_FILM);
					break;
				case 25:
					timeCode.nType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_EBU_25FPS);
					break;
				case 29:
					timeCode.nType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_DF);
					break;
				case 30:
					__attribute__ ((fallthrough));
					/* no break */
				default:
					timeCode.nType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS);
					break;
				}
			} else {
				auto nTime = *reinterpret_cast<uint32_t *>(m_pReceiveBuffer + m_nLxTimeOffset);

				const auto nHours = nTime / 3600000U;
				nTime -= nHours * 3600000U;
				const auto nMinutes = nTime / 60000U;
				nTime -= nMinutes * 60000U;
				const auto nSeconds = nTime / 1000U;
				const auto nMillis = nTime - nSeconds * 1000U;
				const auto nFrames = (nMillis * TCNetConst::FPS[static_cast<uint32_t>(m_TimeCodeType)]) / 1000U;

				timeCode.nFrames = static_cast<uint8_t>(nFrames);
				timeCode.nSeconds = static_cast<uint8_t>(nSeconds);
				timeCode.nMinutes = static_cast<uint8_t>(nMinutes);
				timeCode.nHours = static_cast<uint8_t>(nHours);
				timeCode.nType = static_cast<uint8_t>(m_TimeCodeType);
			}

			assert(m_TCNetTimeCodeCallbackFunctionPtr != nullptr);

			const auto *pSrc = reinterpret_cast<uint8_t *>(&timeCode);
			auto *pDst = reinterpret_cast<uint8_t *>(&m_timeCode);
			auto doSend = false;

			for (uint32_t nIndex = 0; nIndex < sizeof(struct tcnet::TimeCode); nIndex++) {
				doSend |= (*pSrc != *pDst);
				*pDst++ = *pSrc++;
			}

			if (doSend) {
				m_TCNetTimeCodeCallbackFunctionPtr(&timeCode);
			}
		}
#endif
	}

	void HandlePort60002Incoming() {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void HandlePortUnicastIncoming() {
		DEBUG_ENTRY
#ifndef NDEBUG
		const auto *pPacket = reinterpret_cast<struct TTCNetPacketManagementHeader *>(m_pReceiveBuffer);
		const auto messageType  = static_cast<TTCNetMessageType>(pPacket->MessageType);
#endif
		DEBUG_PRINTF("MessageType = %d", static_cast<int>(messageType));
		DEBUG_EXIT
	}

	void HandleOptInOutgoing() {
		m_PacketOptIn.ManagementHeader.SEQ++;
		m_PacketOptIn.ManagementHeader.TimeStamp = Hardware::Get()->Micros();
		m_PacketOptIn.Uptime = static_cast<uint16_t>(Hardware::Get()->GetUpTime());

		Network::Get()->SendTo(m_aHandles[0], &m_PacketOptIn, sizeof(struct TTCNetPacketOptIn), Network::Get()->GetBroadcastIp(), TCNetBroadcast::PORT_0);
	}

	void DumpManagementHeader();
	void DumpOptIn();

private:
	struct TCNetBroadcast {
		static constexpr uint16_t PORT_0 = 60000;
		static constexpr uint16_t PORT_1 = 60001;
		static constexpr uint16_t PORT_2 = 60002;
	};

	struct TCNETUnicast {
		static constexpr uint16_t PORT = 65023;
	};

	int32_t m_aHandles[4];
	uint8_t *m_pReceiveBuffer { nullptr };
	uint32_t m_nIpAddressFrom;
	uint32_t m_nCurrentMillis { 0 };
	uint32_t m_nPreviousMillis { 0 };
	uint32_t m_nLxTimeOffset { 0 };
	uint32_t m_nLxTimeCodeOffset { 0 };

	tcnet::TimeCode m_timeCode = { 0,0,0, 0, UINT8_MAX };

	TCNetTimeCodeCallbackFunctionPtr m_TCNetTimeCodeCallbackFunctionPtr { nullptr };

	float m_fTypeDivider { 1000.0F / 30 };
	tcnet::Layer m_Layer { tcnet::Layer::LAYER_M };
	tcnet::TimeCodeType m_TimeCodeType {tcnet::TimeCodeType::TIMECODE_TYPE_INVALID};
	bool m_bUseTimeCode { false };
	uint8_t m_nSeqTimeMessage { 0 };

	TTCNetPacketOptIn m_PacketOptIn;

	static inline TCNet *s_pThis;
};

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC pop_options
#endif
#if defined (_NDEBUG)
# undef _NDEBUG
# define NDEBUG
#endif
#endif /* TCNET_H_ */
