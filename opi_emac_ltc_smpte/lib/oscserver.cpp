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

#include "tcnetdisplay.h"
#include "ltcdisplayws28xx.h"

#include "oscserver.h"
#include "osc.h"
#include "oscmessage.h"

#include "network.h"

#include "h3/ltcgenerator.h"
#include "h3/systimereader.h"

#include "tcnet.h"

#include "debug.h"

namespace UDP {
	static constexpr auto PORT_DEFAULT = 8000;
	static constexpr auto MAX_BUFFER = 1024;
}

namespace Cmd {
	static constexpr char START[] = "start";
	static constexpr char STOP[] = "stop";
	static constexpr char RESUME[] = "resume";
	static constexpr char RATE[] = "rate";
	static constexpr char SET[] = "/set/";
	static constexpr char GOTO[] = "goto";
	static constexpr char DIRECTION[] = "direction";
	static constexpr char PITCH[] = "pitch";
}

namespace Length {
	static constexpr auto START = sizeof(Cmd::START) - 1;
	static constexpr auto STOP = sizeof(Cmd::STOP) - 1;
	static constexpr auto RESUME = sizeof(Cmd::RESUME) - 1;
	static constexpr auto RATE = sizeof(Cmd::RATE) - 1;
	static constexpr auto SET = sizeof(Cmd::SET) - 1;
	static constexpr auto GOTO = sizeof(Cmd::GOTO) - 1;
	static constexpr auto DIRECTION = sizeof(Cmd::DIRECTION) - 1;
	static constexpr auto PITCH = sizeof(Cmd::PITCH) - 1;

}

namespace TCNET {
	namespace Cmd {
		static constexpr char PATH[] = "tcnet/";
		static constexpr char LAYER[] = "layer/";
		static constexpr char TYPE[] = "type";
		static constexpr char TIMECODE[] = "timecode";
	}
	namespace Length {
		static constexpr auto PATH = sizeof(Cmd::PATH) - 1;
		static constexpr auto LAYER = sizeof(Cmd::LAYER) - 1;
		static constexpr auto TYPE = sizeof(Cmd::TYPE) - 1;
		static constexpr auto TIMECODE = sizeof(Cmd::TIMECODE) - 1;
	}
}

namespace WS28XX {
	namespace Cmd {
		static constexpr char PATH[] = "ws28xx/";
		static constexpr char MASTER[] = "master";
		static constexpr char MESSAGE[] = "message";
	}
	namespace Length {
		static constexpr auto PATH = sizeof(Cmd::PATH) - 1;
		static constexpr auto MASTER = sizeof(Cmd::MASTER) - 1;
		static constexpr auto MESSAGE = sizeof(Cmd::MESSAGE) - 1;
	}
	namespace RGB {
		namespace Cmd {
			static constexpr char PATH[] = "rgb/";
			static constexpr char TIME[] = "time";
			static constexpr char COLON[] = "colon";
			static constexpr char MESSAGE[] = "message";
		}
		namespace Length {
			static constexpr auto PATH = sizeof(Cmd::PATH) - 1;
			static constexpr auto TIME = sizeof(Cmd::TIME) - 1;
			static constexpr auto COLON = sizeof(Cmd::COLON) - 1;
			static constexpr auto MESSAGE = sizeof(Cmd::MESSAGE) - 1;
		}
	}
}

// "hh/mm/ss/ff" -> length = 11
static constexpr auto VALUE_LENGTH = 11;
static constexpr auto RATE_VALUE_LENGTH = 2;

OSCServer::OSCServer(void):
	m_nPortIncoming(UDP::PORT_DEFAULT),
	m_nHandle(-1),
	m_nRemoteIp(0),
	m_nRemotePort(0),
	m_nPathLength(0)
{
	m_pBuffer = new char[UDP::MAX_BUFFER];
	assert(m_pBuffer != 0);

	m_nPathLength = static_cast<uint32_t>(snprintf(m_aPath, sizeof(m_aPath) - 1, "/%s/tc/*", Network::Get()->GetHostName()) - 1);

	DEBUG_PRINTF("%d [%s]", m_nPathLength, m_aPath);
}

OSCServer::~OSCServer(void) {
	delete[] m_pBuffer;
	m_pBuffer = 0;
}

void OSCServer::Start(void) {
	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);
}

void OSCServer::Stop(void) {
}

void OSCServer::Run(void) {
	const uint16_t nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, UDP::MAX_BUFFER, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((nBytesReceived <= 4), 1)) {
		return;
	}

	if (OSC::isMatch(m_pBuffer, m_aPath)) {
		const uint32_t nCommandLength = strlen(m_pBuffer);

		DEBUG_PUTS(m_pBuffer);
		DEBUG_PRINTF("%d,%d %s", static_cast<int>(nCommandLength), m_nPathLength, &m_pBuffer[m_nPathLength]);

		// */pitch f
		if (memcmp(&m_pBuffer[m_nPathLength], Cmd::PITCH, Length::PITCH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const float fValue = Msg.GetFloat(0);

			DEBUG_PRINTF("fValue=%f", fValue);

			LtcGenerator::Get()->ActionSetPitch(fValue);

			DEBUG_PUTS("ActionSetPitch");

			return;
		}
		// */start*
		if (memcmp(&m_pBuffer[m_nPathLength], Cmd::START, Length::START) == 0) {
			if ((nCommandLength == (m_nPathLength + Length::START)) ) {

				LtcGenerator::Get()->ActionStart();
				SystimeReader::Get()->ActionStart();

				DEBUG_PUTS("ActionStart");
			} else if ((nCommandLength == (m_nPathLength + Length::START + 1 + VALUE_LENGTH))) {
				if (m_pBuffer[m_nPathLength + Length::START] == '/') {
					const uint32_t nOffset = m_nPathLength + Length::START + 1;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStart(&m_pBuffer[nOffset]);
					LtcGenerator::Get()->ActionStop();
					LtcGenerator::Get()->ActionStart();

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			} else if ((nCommandLength == (m_nPathLength + Length::START + Length::SET + VALUE_LENGTH))) {
				if (memcmp(&m_pBuffer[m_nPathLength + Length::START], Cmd::SET, Length::SET) == 0) {
					const uint32_t nOffset = m_nPathLength + Length::START + Length::SET;
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
		if (memcmp(&m_pBuffer[m_nPathLength], Cmd::STOP, Length::STOP) == 0) {
			if ((nCommandLength == (m_nPathLength + Length::STOP))) {

				LtcGenerator::Get()->ActionStop();
				SystimeReader::Get()->ActionStop();

				DEBUG_PUTS("ActionStop");
			} else if ((nCommandLength == (m_nPathLength + Length::STOP + Length::SET + VALUE_LENGTH))) {
				if (memcmp(&m_pBuffer[m_nPathLength + Length::STOP], Cmd::SET, Length::SET) == 0) {
					const uint32_t nOffset = m_nPathLength + Length::STOP + Length::SET;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStop(&m_pBuffer[nOffset]);

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			}

			return;
		}
		// */set/*
		if ((nCommandLength == (m_nPathLength + Length::RATE + Length::SET + RATE_VALUE_LENGTH))) {
			if (memcmp(&m_pBuffer[m_nPathLength + Length::RATE], Cmd::SET, Length::SET) == 0) {
				const uint32_t nOffset = m_nPathLength + Length::RATE + Length::SET;

				LtcGenerator::Get()->ActionSetRate(&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
			}

			return;
		}
		// */resume
		if ( (nCommandLength == (m_nPathLength + Length::RESUME)) && (memcmp(&m_pBuffer[m_nPathLength], Cmd::RESUME, Length::RESUME) == 0)) {
			LtcGenerator::Get()->ActionResume();

			DEBUG_PUTS("ActionResume");

			return;
		}
		// */goto/*
		if ((nCommandLength == (m_nPathLength + Length::GOTO + 1 + VALUE_LENGTH)) && (memcmp(&m_pBuffer[m_nPathLength], Cmd::GOTO, Length::GOTO) == 0)) {
			if (m_pBuffer[m_nPathLength + Length::GOTO] == '/') {
				const uint32_t nOffset = m_nPathLength + Length::GOTO + 1;
				m_pBuffer[nOffset + 2] = ':';
				m_pBuffer[nOffset + 5] = ':';
				m_pBuffer[nOffset + 8] = '.';

				LtcGenerator::Get()->ActionGoto(&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
			}

			return;
		}
		// */direction/*
		if ((nCommandLength <= (m_nPathLength + Length::DIRECTION + 1 + 8)) && (memcmp(&m_pBuffer[m_nPathLength], Cmd::DIRECTION, Length::DIRECTION) == 0)) {
			if (m_pBuffer[m_nPathLength + Length::DIRECTION] == '/') {
				const uint32_t nOffset = m_nPathLength + Length::DIRECTION + 1;
				LtcGenerator::Get()->ActionSetDirection(&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
			}

			return;
		}

		// */tcnet/
		if (memcmp(&m_pBuffer[m_nPathLength], TCNET::Cmd::PATH, TCNET::Length::PATH) == 0) {
			// layer/?
			if (nCommandLength == (m_nPathLength + TCNET::Length::PATH + TCNET::Length::LAYER + 1)) {
				if (memcmp(&m_pBuffer[m_nPathLength + TCNET::Length::PATH], TCNET::Cmd::LAYER, TCNET::Length::LAYER) == 0) {
					const uint32_t nOffset = m_nPathLength + TCNET::Length::PATH + TCNET::Length::LAYER;
					const TCNetLayer tLayer = TCNet::GetLayer(m_pBuffer[nOffset]);

					TCNet::Get()->SetLayer(tLayer);
					TCNetDisplay::Show();

					DEBUG_PRINTF("*/tcnet/layer/%c -> %d", m_pBuffer[nOffset], static_cast<int>(tLayer));
				}

				return;
			}
			// type i
			if (nCommandLength == (m_nPathLength + TCNET::Length::PATH + TCNET::Length::TYPE)) {
				if (memcmp(&m_pBuffer[m_nPathLength + TCNET::Length::PATH], TCNET::Cmd::TYPE, TCNET::Length::TYPE) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

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
				}

				return;
			}
			// timecode i
			if (nCommandLength == (m_nPathLength + TCNET::Length::PATH + TCNET::Length::TIMECODE)) {
				if (memcmp(&m_pBuffer[m_nPathLength + TCNET::Length::PATH], TCNET::Cmd::TYPE, TCNET::Length::TIMECODE) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

					const int nValue = Msg.GetInt(0);
					const bool bUseTimeCode = (nValue > 0);

					TCNet::Get()->SetUseTimeCode(bUseTimeCode);
					TCNetDisplay::Show();

					DEBUG_PRINTF("*/tcnet/timecode -> %d", static_cast<int>(bUseTimeCode));
				}

				return;
			}

			return;
		}

		// */ws28xx/
		if (memcmp(&m_pBuffer[m_nPathLength], WS28XX::Cmd::PATH, WS28XX::Length::PATH) == 0) {
			// ws28xx/master i
			if (nCommandLength == (m_nPathLength + WS28XX::Length::PATH + WS28XX::Length::MASTER)) {
				if (memcmp(&m_pBuffer[m_nPathLength + WS28XX::Length::PATH], WS28XX::Cmd::MASTER, WS28XX::Length::MASTER) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

					const int nValue = Msg.GetInt(0);

					LtcDisplayWS28xx::Get()->SetMaster(nValue);

					DEBUG_PRINTF("*/ws28xx/master -> %d", static_cast<int>(static_cast<uint8_t>(nValue)));
				}

				return;
			}
			// ws28xx/message string
			if (nCommandLength == (m_nPathLength + WS28XX::Length::PATH + WS28XX::Length::MESSAGE)) {
				if (memcmp(&m_pBuffer[m_nPathLength + WS28XX::Length::PATH], WS28XX::Cmd::MESSAGE, WS28XX::Length::MESSAGE) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

					char *pString = Msg.GetString(0);
					const uint8_t nSize = strlen(pString);

					LtcDisplayWS28xx::Get()->SetMessage(pString, nSize);

					DEBUG_PRINTF("*/ws28xx/message -> [%.*s]", nSize, pString);
				}

				return;
			}
			// ws28xx/rgb/*
			if (nCommandLength > (m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + WS28XX::Length::PATH], WS28XX::RGB::Cmd::PATH, WS28XX::RGB::Length::PATH) == 0) {
					// ws28xx/rgb/time iii
					if (nCommandLength == (m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH + WS28XX::RGB::Length::TIME)) {
						if (memcmp(&m_pBuffer[m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH], WS28XX::RGB::Cmd::TIME, WS28XX::RGB::Length::TIME) == 0) {
							SetWS28xxRGB(nBytesReceived, LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT);
						}

						return;
					}
					// ws28xx/rgb/colon iii
					if (nCommandLength == (m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH + WS28XX::RGB::Length::COLON)) {
						if (memcmp(&m_pBuffer[m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH], WS28XX::RGB::Cmd::COLON, WS28XX::RGB::Length::COLON) == 0) {
							SetWS28xxRGB(nBytesReceived, LTCDISPLAYWS28XX_COLOUR_INDEX_COLON);
						}

						return;
					}
					// ws28xx/rgb/message iii
					if (nCommandLength == (m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH + WS28XX::RGB::Length::MESSAGE)) {
						if (memcmp(&m_pBuffer[m_nPathLength + WS28XX::Length::PATH + WS28XX::RGB::Length::PATH], WS28XX::RGB::Cmd::MESSAGE, WS28XX::RGB::Length::MESSAGE) == 0) {
							SetWS28xxRGB(nBytesReceived, LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE);
						}

						return;
					}
				}

				return;
			}
		}
	}
}

void OSCServer::SetWS28xxRGB(uint32_t nSize, TLtcDisplayWS28xxColourIndex tIndex) {
	OSCMessage Msg(m_pBuffer, nSize);

	if (Msg.GetArgc() == 3) {
		const int nRed = Msg.GetInt(0);
		const int nGreen = Msg.GetInt(1);
		const int nBlue = Msg.GetInt(2);

		LtcDisplayWS28xx::Get()->SetRGB(nRed, nGreen, nBlue, tIndex);

		DEBUG_PRINTF("*/ws28xx/rgb/[%d] -> %d %d %d", static_cast<int>(tIndex), static_cast<int>(nRed), static_cast<int>(nGreen), static_cast<int>(nBlue));
	} else {
		DEBUG_PUTS("Invalid ws28xx/rgb/*");
	}
}

void OSCServer::Print(void) {
	printf("OSC Server\n");
	printf(" Port : %d\n", m_nPortIncoming);
	printf(" Path : [%s]\n", m_aPath);
}
