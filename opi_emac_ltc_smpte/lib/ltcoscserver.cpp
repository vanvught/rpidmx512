/**
 * @file oscserver.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ltcdisplayrgb.h>

#include "ltcoscserver.h"

#include "tcnetdisplay.h"
#include "osc.h"
#include "oscsimplemessage.h"

#include "network.h"

#include "h3/ltcgenerator.h"
#include "h3/systimereader.h"

#include "tcnet.h"

#include "debug.h"

namespace udp {
static constexpr auto MAX_BUFFER = 1024;
} // namespace udp

namespace cmd {
static constexpr char START[] = "start";
static constexpr char STOP[] = "stop";
static constexpr char RESUME[] = "resume";
static constexpr char RATE[] = "rate";
static constexpr char SET[] = "/set/";
static constexpr char GOTO[] = "goto";
static constexpr char DIRECTION[] = "direction";
static constexpr char PITCH[] = "pitch";
static constexpr char FORWARD[] = "forward";
static constexpr char BACKWARD[] = "backward";
} // namespace cmd

namespace length {
static constexpr auto START = sizeof(cmd::START) - 1;
static constexpr auto STOP = sizeof(cmd::STOP) - 1;
static constexpr auto RESUME = sizeof(cmd::RESUME) - 1;
static constexpr auto RATE = sizeof(cmd::RATE) - 1;
static constexpr auto SET = sizeof(cmd::SET) - 1;
static constexpr auto GOTO = sizeof(cmd::GOTO) - 1;
static constexpr auto DIRECTION = sizeof(cmd::DIRECTION) - 1;
static constexpr auto PITCH = sizeof(cmd::PITCH) - 1;
static constexpr auto FORWARD = sizeof(cmd::FORWARD) - 1;
static constexpr auto BACKWARD = sizeof(cmd::BACKWARD) - 1;
} // namespace length

namespace tcnet {
namespace cmd {
static constexpr char PATH[] = "tcnet/";
static constexpr char LAYER[] = "layer/";
static constexpr char TYPE[] = "type";
static constexpr char TIMECODE[] = "timecode";
} // namespace cmd
namespace length {
static constexpr auto PATH = sizeof(cmd::PATH) - 1;
static constexpr auto LAYER = sizeof(cmd::LAYER) - 1;
static constexpr auto TYPE = sizeof(cmd::TYPE) - 1;
static constexpr auto TIMECODE = sizeof(cmd::TIMECODE) - 1;
} // namespace length
} // namespace tnet

namespace ws28xx {
namespace cmd {
static constexpr char PATH[] = "ws28xx/";
static constexpr char MASTER[] = "master";
static constexpr char MESSAGE[] = "message";
static constexpr char INFO[] = "info";
}  // namespace cmd
namespace length {
static constexpr auto PATH = sizeof(cmd::PATH) - 1;
static constexpr auto MASTER = sizeof(cmd::MASTER) - 1;
static constexpr auto MESSAGE = sizeof(cmd::MESSAGE) - 1;
static constexpr auto INFO = sizeof(cmd::INFO) - 1;
} // namespace length
namespace rgb {
namespace cmd {
static constexpr char PATH[] = "rgb/";
static constexpr char TIME[] = "time";
static constexpr char COLON[] = "colon";
static constexpr char MESSAGE[] = "message";
static constexpr char FPS[] = "fps";
static constexpr char INFO[] = "info";
} // namespace cmd
namespace length {
static constexpr auto PATH = sizeof(cmd::PATH) - 1;
static constexpr auto TIME = sizeof(cmd::TIME) - 1;
static constexpr auto COLON = sizeof(cmd::COLON) - 1;
static constexpr auto MESSAGE = sizeof(cmd::MESSAGE) - 1;
static constexpr auto FPS = sizeof(cmd::FPS) - 1;
static constexpr auto INFO = sizeof(cmd::INFO) - 1;
} // namespace length
} // namespace rgb
} // namespace ws28xx

// "hh/mm/ss/ff" -> length = 11
static constexpr auto VALUE_LENGTH = 11;
static constexpr auto FPS_VALUE_LENGTH = 2;

LtcOscServer::LtcOscServer():
	m_nPortIncoming(osc::port::DEFAULT_INCOMING)
	
{
	m_pBuffer = new char[udp::MAX_BUFFER];
	assert(m_pBuffer != nullptr);

	m_nPathLength = static_cast<uint32_t>(snprintf(m_aPath, sizeof(m_aPath) - 1, "/%s/tc/*", Network::Get()->GetHostName()) - 1);

	DEBUG_PRINTF("%d [%s]", m_nPathLength, m_aPath);
}

LtcOscServer::~LtcOscServer() {
	delete[] m_pBuffer;
	m_pBuffer = nullptr;
}

void LtcOscServer::Start() {
	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);
}

void LtcOscServer::Stop() {
}

void LtcOscServer::Run() {
	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, udp::MAX_BUFFER, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((nBytesReceived <= 4), 1)) {
		return;
	}

	if (OSC::isMatch(m_pBuffer, m_aPath)) {
		const auto nCommandLength = strlen(m_pBuffer);

		DEBUG_PRINTF("[%s]:%d %d:|%s|", m_pBuffer, static_cast<int>(nCommandLength), m_nPathLength, &m_pBuffer[m_nPathLength]);

		// */pitch f
		if (memcmp(&m_pBuffer[m_nPathLength], cmd::PITCH, length::PITCH) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			if (Msg.GetType(0) != osc::type::FLOAT) {
				return;
			}

			const auto fValue = Msg.GetFloat(0);

			DEBUG_PRINTF("fValue=%f", fValue);

			LtcGenerator::Get()->ActionSetPitch(fValue);

			DEBUG_PUTS("ActionSetPitch");
			return;
		}
		// */start*
		if (memcmp(&m_pBuffer[m_nPathLength], cmd::START, length::START) == 0) {
			if ((nCommandLength == (m_nPathLength + length::START)) ) {

				LtcGenerator::Get()->ActionStart();
				SystimeReader::Get()->ActionStart();

				DEBUG_PUTS("ActionStart");
			} else if ((nCommandLength == (m_nPathLength + length::START + 1 + VALUE_LENGTH))) {
				if (m_pBuffer[m_nPathLength + length::START] == '/') {
					const auto nOffset = m_nPathLength + length::START + 1;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStart(&m_pBuffer[nOffset]);
					LtcGenerator::Get()->ActionStop();
					LtcGenerator::Get()->ActionStart();

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			} else if ((nCommandLength == (m_nPathLength + length::START + length::SET + VALUE_LENGTH))) {
				if (memcmp(&m_pBuffer[m_nPathLength + length::START], cmd::SET, length::SET) == 0) {
					const auto nOffset = m_nPathLength + length::START + length::SET;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStart(&m_pBuffer[nOffset]);

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			}
			return;
		}
		// */stop*
		if (memcmp(&m_pBuffer[m_nPathLength], cmd::STOP, length::STOP) == 0) {
			if ((nCommandLength == (m_nPathLength + length::STOP))) {

				LtcGenerator::Get()->ActionStop();
				SystimeReader::Get()->ActionStop();

				DEBUG_PUTS("ActionStop");
			} else if ((nCommandLength == (m_nPathLength + length::STOP + length::SET + VALUE_LENGTH))) {
				if (memcmp(&m_pBuffer[m_nPathLength + length::STOP], cmd::SET, length::SET) == 0) {
					const auto nOffset = m_nPathLength + length::STOP + length::SET;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStop(&m_pBuffer[nOffset]);

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			}

			return;
		}
		// */forward i
		if (memcmp(&m_pBuffer[m_nPathLength], cmd::FORWARD, length::FORWARD) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			if (Msg.GetType(0) != osc::type::INT32) {
				return;
			}

			const auto nValue = Msg.GetInt(0);

			if (nValue > 0) {
				LtcGenerator::Get()->ActionForward(nValue);
				DEBUG_PRINTF("ActionForward(%d)", nValue);
			}
			return;
		}
		// */backward i
		if (memcmp(&m_pBuffer[m_nPathLength], cmd::BACKWARD, length::BACKWARD) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			if (Msg.GetType(0) != osc::type::INT32) {
				return;
			}

			const auto nValue = Msg.GetInt(0);

			if (nValue > 0) {
				LtcGenerator::Get()->ActionBackward(nValue);
				DEBUG_PRINTF("ActionBackward(%d)", nValue);
			}
			return;
		}
		// */set/*
		if ((nCommandLength == (m_nPathLength + length::RATE + length::SET + FPS_VALUE_LENGTH))) {
			if (memcmp(&m_pBuffer[m_nPathLength + length::RATE], cmd::SET, length::SET) == 0) {
				const auto nOffset = m_nPathLength + length::RATE + length::SET;

				LtcGenerator::Get()->ActionSetRate(&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
				return;
			}
		}
		// */resume
		if ( (nCommandLength == (m_nPathLength + length::RESUME)) && (memcmp(&m_pBuffer[m_nPathLength], cmd::RESUME, length::RESUME) == 0)) {
			LtcGenerator::Get()->ActionResume();

			DEBUG_PUTS("ActionResume");
			return;
		}
		// */goto/*
		if ((nCommandLength == (m_nPathLength + length::GOTO + 1 + VALUE_LENGTH)) && (memcmp(&m_pBuffer[m_nPathLength], cmd::GOTO, length::GOTO) == 0)) {
			if (m_pBuffer[m_nPathLength + length::GOTO] == '/') {
				const auto nOffset = m_nPathLength + length::GOTO + 1;
				m_pBuffer[nOffset + 2] = ':';
				m_pBuffer[nOffset + 5] = ':';
				m_pBuffer[nOffset + 8] = '.';

				LtcGenerator::Get()->ActionGoto(&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
				return;
			}
		}
		// */direction/*
		if ((nCommandLength <= (m_nPathLength + length::DIRECTION + 1 + 8)) && (memcmp(&m_pBuffer[m_nPathLength], cmd::DIRECTION, length::DIRECTION) == 0)) {
			if (m_pBuffer[m_nPathLength + length::DIRECTION] == '/') {
				const uint32_t nOffset = m_nPathLength + length::DIRECTION + 1;
				LtcGenerator::Get()->ActionSetDirection(&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
				return;
			}
		}

		// */tcnet/
		if (memcmp(&m_pBuffer[m_nPathLength], tcnet::cmd::PATH, tcnet::length::PATH) == 0) {
			// layer/?
			if (nCommandLength == (m_nPathLength + tcnet::length::PATH + tcnet::length::LAYER + 1)) {
				if (memcmp(&m_pBuffer[m_nPathLength + tcnet::length::PATH], tcnet::cmd::LAYER, tcnet::length::LAYER) == 0) {
					const auto nOffset = m_nPathLength + tcnet::length::PATH + tcnet::length::LAYER;
					const auto tLayer = TCNet::GetLayer(m_pBuffer[nOffset]);

					TCNet::Get()->SetLayer(tLayer);
					TCNetDisplay::Show();

					DEBUG_PRINTF("*/tcnet/layer/%c -> %d", m_pBuffer[nOffset], static_cast<int>(tLayer));
					return;
				}
			}
			// type i
			if (nCommandLength == (m_nPathLength + tcnet::length::PATH + tcnet::length::TYPE)) {
				if (memcmp(&m_pBuffer[m_nPathLength + tcnet::length::PATH], tcnet::cmd::TYPE, tcnet::length::TYPE) == 0) {
					OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

					if (Msg.GetType(0) != osc::type::INT32) {
						return;
					}

					const int nValue = Msg.GetInt(0);

					switch (nValue) {
					case 24:
						TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_FILM);
						TCNetDisplay::Show();
						break;
					case 25:
						TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_EBU_25FPS);
						TCNetDisplay::Show();
						break;
					case 29:
						TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_DF);
						TCNetDisplay::Show();
						break;
					case 30:
						TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_SMPTE_30FPS);
						TCNetDisplay::Show();
						break;
					default:
						break;
					}

					DEBUG_PRINTF("*/tcnet/type -> %d", nValue);
					return;
				}
			}
			// timecode i
			if (nCommandLength == (m_nPathLength + tcnet::length::PATH + tcnet::length::TIMECODE)) {
				if (memcmp(&m_pBuffer[m_nPathLength + tcnet::length::PATH], tcnet::cmd::TYPE, tcnet::length::TIMECODE) == 0) {
					OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

					if (Msg.GetType(0) != osc::type::INT32) {
						return;
					}

					const auto nValue = Msg.GetInt(0);
					const auto bUseTimeCode = (nValue > 0);

					TCNet::Get()->SetUseTimeCode(bUseTimeCode);
					TCNetDisplay::Show();

					DEBUG_PRINTF("*/tcnet/timecode -> %d", static_cast<int>(bUseTimeCode));
					return;
				}
			}
		}

		// */ws28xx/
		if (memcmp(&m_pBuffer[m_nPathLength], ws28xx::cmd::PATH, ws28xx::length::PATH) == 0) {
			// ws28xx/master i
			if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::length::MASTER)) {
				if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH], ws28xx::cmd::MASTER, ws28xx::length::MASTER) == 0) {
					OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

					if (Msg.GetType(0) != osc::type::INT32) {
						return;
					}

					const auto nValue = Msg.GetInt(0);

					LtcDisplayRgb::Get()->SetMaster(nValue);

					DEBUG_PRINTF("*/ws28xx/master -> %d", static_cast<int>(static_cast<uint8_t>(nValue)));
					return;
				}
			}
			// ws28xx/message string
			if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::length::MESSAGE)) {
				if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH], ws28xx::cmd::MESSAGE, ws28xx::length::MESSAGE) == 0) {
					OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

					if (Msg.GetType(0) != osc::type::STRING) {
						return;
					}

					const auto *pString = Msg.GetString(0);
					const auto nSize = strlen(pString);

					LtcDisplayRgb::Get()->SetMessage(pString, nSize);

					DEBUG_PRINTF("*/ws28xx/message -> [%.*s]", nSize, pString);
					return;
				}
			}
			// ws28xx/info string
			if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::length::INFO)) {
				if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH], ws28xx::cmd::INFO, ws28xx::length::INFO) == 0) {
					OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

					if (Msg.GetType(0) != osc::type::STRING) {
						return;
					}

					const auto *pString = Msg.GetString(0);

					LtcDisplayRgb::Get()->ShowInfo(pString);

					DEBUG_PRINTF("*/ws28xx/info -> [%.*s]", 8, pString);
					return;
				}

			}
			// ws28xx/rgb/*
			if (nCommandLength > (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH], ws28xx::rgb::cmd::PATH, ws28xx::rgb::length::PATH) == 0) {
					// ws28xx/rgb/time iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::TIME)) {
						if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::TIME, ws28xx::rgb::length::TIME) == 0) {
							SetWS28xxRGB(nBytesReceived, ltcdisplayrgb::ColourIndex::TIME);
							return;
						}
					}
					// ws28xx/rgb/colon iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::COLON)) {
						if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::COLON, ws28xx::rgb::length::COLON) == 0) {
							SetWS28xxRGB(nBytesReceived, ltcdisplayrgb::ColourIndex::COLON);
							return;
						}
					}
					// ws28xx/rgb/message iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::MESSAGE)) {
						if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::MESSAGE, ws28xx::rgb::length::MESSAGE) == 0) {
							SetWS28xxRGB(nBytesReceived, ltcdisplayrgb::ColourIndex::MESSAGE);
							return;
						}
					}
					// ws28xx/rgb/fps iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::FPS)) {
						if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::FPS, ws28xx::rgb::length::FPS) == 0) {
							SetWS28xxRGB(nBytesReceived, ltcdisplayrgb::ColourIndex::FPS);
							return;
						}
					}
					// ws28xx/rgb/info iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::INFO)) {
						if (memcmp(&m_pBuffer[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::INFO, ws28xx::rgb::length::INFO) == 0) {
							SetWS28xxRGB(nBytesReceived, ltcdisplayrgb::ColourIndex::INFO);
							return;
						}
					}
					return;
				}
			}
		}
	}
}

void LtcOscServer::SetWS28xxRGB(uint32_t nSize, ltcdisplayrgb::ColourIndex tIndex) {
	OscSimpleMessage Msg(m_pBuffer, nSize);

	if (Msg.GetArgc() == 3) {
		const auto nRed = Msg.GetInt(0);
		const auto nGreen = Msg.GetInt(1);
		const auto nBlue = Msg.GetInt(2);

		LtcDisplayRgb::Get()->SetRGB(nRed, nGreen, nBlue, tIndex);

		DEBUG_PRINTF("*/ws28xx/rgb/[%d] -> %d %d %d", static_cast<int>(tIndex), static_cast<int>(nRed), static_cast<int>(nGreen), static_cast<int>(nBlue));
	} else {
		DEBUG_PUTS("Invalid ws28xx/rgb/*");
	}
}

void LtcOscServer::Print() {
	printf("OSC Server\n");
	printf(" Port : %d\n", m_nPortIncoming);
	printf(" Path : [%s]\n", m_aPath);
}
