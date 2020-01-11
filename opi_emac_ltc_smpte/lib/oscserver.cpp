/**
 * @file oscserver.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

enum TOscServerPort {
	OSCSERVER_PORT_DEFAULT_INCOMING = 8000
};

#define OSCSERVER_MAX_BUFFER 		1024

// Internal

static const char sStart[] ALIGNED = "start";
#define START_LENGTH 			(sizeof(sStart)/sizeof(sStart[0]) - 1)

static const char sStop[] ALIGNED = "stop";
#define STOP_LENGTH 			(sizeof(sStop)/sizeof(sStop[0]) - 1)

static const char sResume[] ALIGNED = "resume";
#define RESUME_LENGTH 			(sizeof(sResume)/sizeof(sResume[0]) - 1)

static const char sRate[] ALIGNED = "rate";
#define RATE_LENGTH 			(sizeof(sRate)/sizeof(sRate[0]) - 1)

static const char sSet[] ALIGNED = "/set/";
#define SET_LENGTH 				(sizeof(sSet)/sizeof(sSet[0]) - 1)

static const char sGoto[] ALIGNED = "goto";
#define GOTO_LENGTH 			(sizeof(sGoto)/sizeof(sGoto[0]) - 1)

static const char sDirection[] ALIGNED = "direction";
#define DIRECTION_LENGTH 		(sizeof(sDirection)/sizeof(sDirection[0]) - 1)

static const char sPitch[] ALIGNED = "pitch";
#define PITCH_LENGTH 			(sizeof(sPitch)/sizeof(sPitch[0]) - 1)

// TCNet

static const char sTCNet[] ALIGNED = "tcnet/";
#define TCNET_LENGTH 			(sizeof(sTCNet)/sizeof(sTCNet[0]) - 1)

static const char sTCNetLayer[] ALIGNED = "layer/";
#define TCNETLAYER_LENGTH 		(sizeof(sTCNetLayer)/sizeof(sTCNetLayer[0]) - 1)

static const char sTCNetType[] ALIGNED = "type";
#define TCNETTYPE_LENGTH 		(sizeof(sTCNetType)/sizeof(sTCNetType[0]) - 1)

static const char sTCNetTimeCode[] ALIGNED = "timecode";
#define TCNETTIMECODE_LENGTH 	(sizeof(sTCNetTimeCode)/sizeof(sTCNetTimeCode[0]) - 1)

// WS28xx Display

static const char sWS28xx[] ALIGNED = "ws28xx/";
#define WS28XX_LENGTH 			(sizeof(sWS28xx)/sizeof(sWS28xx[0]) - 1)

static const char sWS28xxMaster[] ALIGNED = "master";
#define WS28XXMASTER_LENGTH 	(sizeof(sWS28xxMaster)/sizeof(sWS28xxMaster[0]) - 1)

static const char sWS28xxMessage[] ALIGNED = "message";
#define WS28XXMESSAGE_LENGTH 	(sizeof(sWS28xxMessage)/sizeof(sWS28xxMessage[0]) - 1)

static const char sWS28xxRgb[] ALIGNED = "rgb/";
#define WS28XXRGB_LENGTH 		(sizeof(sWS28xxRgb)/sizeof(sWS28xxRgb[0]) - 1)

static const char sWS28xxRgbTime[] ALIGNED = "time";
#define WS28XXTIME_LENGTH 		(sizeof(sWS28xxRgbTime)/sizeof(sWS28xxRgbTime[0]) - 1)

static const char sWS28xxRgbColon[] ALIGNED = "colon";
#define WS28XXCOLON_LENGTH 		(sizeof(sWS28xxRgbColon)/sizeof(sWS28xxRgbColon[0]) - 1)

static const char sWS28xxRgbMessage[] ALIGNED = "message";
#define WS28XXRGBMESSAGE_LENGTH (sizeof(sWS28xxRgbMessage)/sizeof(sWS28xxRgbMessage[0]) - 1)

// "hh/mm/ss/ff" -> length = 11
#define VALUE_LENGTH		11
#define RATE_VALUE_LENGTH	2

OSCServer::OSCServer(void):
	m_nPortIncoming(OSCSERVER_PORT_DEFAULT_INCOMING),
	m_nHandle(-1),
	m_nRemoteIp(0),
	m_nRemotePort(0),
	m_nPathLength(0)
{
	m_pBuffer = new uint8_t[OSCSERVER_MAX_BUFFER];
	assert(m_pBuffer != 0);

	m_nPathLength = snprintf(m_aPath, sizeof(m_aPath) - 1, "/%s/tc/*", Network::Get()->GetHostName()) - 1;

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
	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, OSCSERVER_MAX_BUFFER, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((nBytesReceived <= 4), 1)) {
		return;
	}

	if (OSC::isMatch((const char*) m_pBuffer, m_aPath)) {
		const uint32_t nCommandLength = strlen((const char *)m_pBuffer);

		DEBUG_PUTS(m_pBuffer);
		DEBUG_PRINTF("%d,%d %s", (int) nCommandLength, m_nPathLength, &m_pBuffer[m_nPathLength]);

		// */pitch f
		if (memcmp(&m_pBuffer[m_nPathLength], sPitch, PITCH_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const float fValue = Msg.GetFloat(0);

			DEBUG_PRINTF("fValue=%f", fValue);

			LtcGenerator::Get()->ActionSetPitch(fValue);

			DEBUG_PUTS("ActionSetPitch");

			return;
		}
		// */start*
		if (memcmp(&m_pBuffer[m_nPathLength], sStart, START_LENGTH) == 0) {
			if ((nCommandLength == (m_nPathLength + START_LENGTH)) ) {

				LtcGenerator::Get()->ActionStart();
				SystimeReader::Get()->ActionStart();

				DEBUG_PUTS("ActionStart");
			} else if ((nCommandLength == (m_nPathLength + START_LENGTH + 1 + VALUE_LENGTH))) {
				if (m_pBuffer[m_nPathLength + START_LENGTH] == '/') {
					const uint32_t nOffset = m_nPathLength + START_LENGTH + 1;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStart((const char *)&m_pBuffer[nOffset]);
					LtcGenerator::Get()->ActionStop();
					LtcGenerator::Get()->ActionStart();

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			} else if ((nCommandLength == (m_nPathLength + START_LENGTH + SET_LENGTH + VALUE_LENGTH))) {
				if (memcmp(&m_pBuffer[m_nPathLength + START_LENGTH], sSet, SET_LENGTH) == 0) {
					const uint32_t nOffset = m_nPathLength + START_LENGTH + SET_LENGTH;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStart((const char *)&m_pBuffer[nOffset]);

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			}

			return;
		}
		// */stop*
		if (memcmp(&m_pBuffer[m_nPathLength], sStop, STOP_LENGTH) == 0) {
			if ((nCommandLength == (m_nPathLength + STOP_LENGTH))) {

				LtcGenerator::Get()->ActionStop();
				SystimeReader::Get()->ActionStop();

				DEBUG_PUTS("ActionStop");
			} else if ((nCommandLength == (m_nPathLength + STOP_LENGTH + SET_LENGTH + VALUE_LENGTH))) {
				if (memcmp(&m_pBuffer[m_nPathLength + STOP_LENGTH], sSet, SET_LENGTH) == 0) {
					const uint32_t nOffset = m_nPathLength + STOP_LENGTH + SET_LENGTH;
					m_pBuffer[nOffset + 2] = ':';
					m_pBuffer[nOffset + 5] = ':';
					m_pBuffer[nOffset + 8] = '.';

					LtcGenerator::Get()->ActionSetStop((const char *)&m_pBuffer[nOffset]);

					DEBUG_PUTS(&m_pBuffer[nOffset]);
				}
			}

			return;
		}
		// */set/*
		if ((nCommandLength == (m_nPathLength + RATE_LENGTH + SET_LENGTH + RATE_VALUE_LENGTH))) {
			if (memcmp(&m_pBuffer[m_nPathLength + RATE_LENGTH], sSet, SET_LENGTH) == 0) {
				const uint32_t nOffset = m_nPathLength + RATE_LENGTH + SET_LENGTH;

				LtcGenerator::Get()->ActionSetRate((const char *)&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
			}

			return;
		}
		// */resume
		if ( (nCommandLength == (m_nPathLength + RESUME_LENGTH)) && (memcmp(&m_pBuffer[m_nPathLength], sResume, RESUME_LENGTH) == 0)) {
			LtcGenerator::Get()->ActionResume();

			DEBUG_PUTS("ActionResume");

			return;
		}
		// */goto/*
		if ((nCommandLength == (m_nPathLength + GOTO_LENGTH + 1 + VALUE_LENGTH)) && (memcmp(&m_pBuffer[m_nPathLength], sGoto, GOTO_LENGTH) == 0)) {
			if (m_pBuffer[m_nPathLength + GOTO_LENGTH] == '/') {
				const uint32_t nOffset = m_nPathLength + GOTO_LENGTH + 1;
				m_pBuffer[nOffset + 2] = ':';
				m_pBuffer[nOffset + 5] = ':';
				m_pBuffer[nOffset + 8] = '.';

				LtcGenerator::Get()->ActionGoto((const char *)&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
			}

			return;
		}
		// */direction/*
		if ((nCommandLength <= (m_nPathLength + DIRECTION_LENGTH + 1 + 8)) && (memcmp(&m_pBuffer[m_nPathLength], sDirection, DIRECTION_LENGTH) == 0)) {
			if (m_pBuffer[m_nPathLength + DIRECTION_LENGTH] == '/') {
				const uint32_t nOffset = m_nPathLength + DIRECTION_LENGTH + 1;
				LtcGenerator::Get()->ActionSetDirection((const char *)&m_pBuffer[nOffset]);

				DEBUG_PUTS(&m_pBuffer[nOffset]);
			}

			return;
		}

		// */tcnet/
		if (memcmp(&m_pBuffer[m_nPathLength], sTCNet, TCNET_LENGTH) == 0) {
			// layer/?
			if (nCommandLength == (m_nPathLength + TCNET_LENGTH + TCNETLAYER_LENGTH + 1)) {
				if (memcmp(&m_pBuffer[m_nPathLength + TCNET_LENGTH], sTCNetLayer, TCNETLAYER_LENGTH) == 0) {
					const uint32_t nOffset = m_nPathLength + TCNET_LENGTH + TCNETLAYER_LENGTH;
					const TTCNetLayers tLayer = TCNet::GetLayer(m_pBuffer[nOffset]);

					TCNet::Get()->SetLayer(tLayer);
					TCNetDisplay::Show();

					DEBUG_PRINTF("*/tcnet/layer/%c -> %d", m_pBuffer[nOffset], (int) tLayer);
				}

				return;
			}
			// type i
			if (nCommandLength == (m_nPathLength + TCNET_LENGTH + TCNETTYPE_LENGTH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + TCNET_LENGTH], sTCNetType, TCNETTYPE_LENGTH) == 0) {
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
			if (nCommandLength == (m_nPathLength + TCNET_LENGTH + TCNETTIMECODE_LENGTH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + TCNET_LENGTH], sTCNetType, TCNETTIMECODE_LENGTH) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

					const int nValue = Msg.GetInt(0);
					const bool bUseTimeCode = (nValue > 0);

					TCNet::Get()->SetUseTimeCode(bUseTimeCode);
					TCNetDisplay::Show();

					DEBUG_PRINTF("*/tcnet/timecode -> %d", (int) bUseTimeCode);
				}

				return;
			}

			return;
		}

		// */ws28xx/
		if (memcmp(&m_pBuffer[m_nPathLength], sWS28xx, WS28XX_LENGTH) == 0) {
			// ws28xx/master i
			if (nCommandLength == (m_nPathLength + WS28XX_LENGTH + WS28XXMASTER_LENGTH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + WS28XX_LENGTH], sWS28xxMaster, WS28XXMASTER_LENGTH) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

					const int nValue = Msg.GetInt(0);

					LtcDisplayWS28xx::Get()->SetMaster((uint8_t) nValue);

					DEBUG_PRINTF("*/ws28xx/master -> %d", (int)((uint8_t) nValue));
				}

				return;
			}
			// ws28xx/message string
			if (nCommandLength == (m_nPathLength + WS28XX_LENGTH + WS28XXMESSAGE_LENGTH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + WS28XX_LENGTH], sWS28xxMessage, WS28XXMESSAGE_LENGTH) == 0) {
					OSCMessage Msg(m_pBuffer, nBytesReceived);

					char *pString = Msg.GetString(0);
					const uint8_t nSize = strlen(pString);

					LtcDisplayWS28xx::Get()->SetMessage(pString, nSize);

					DEBUG_PRINTF("*/ws28xx/message -> [%.*s]", nSize, pString);
				}

				return;
			}
			// ws28xx/rgb/*
			if (nCommandLength > (m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH)) {
				if (memcmp(&m_pBuffer[m_nPathLength + WS28XX_LENGTH], sWS28xxRgb, WS28XXRGB_LENGTH) == 0) {
					// ws28xx/rgb/time iii
					if (nCommandLength == (m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH + WS28XXTIME_LENGTH)) {
						if (memcmp(&m_pBuffer[m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH], sWS28xxRgbTime, WS28XXTIME_LENGTH) == 0) {
							SetWS28xxRGB(nBytesReceived, LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT);
						}

						return;
					}
					// ws28xx/rgb/colon iii
					if (nCommandLength == (m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH + WS28XXCOLON_LENGTH)) {
						if (memcmp(&m_pBuffer[m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH], sWS28xxRgbColon, WS28XXCOLON_LENGTH) == 0) {
							SetWS28xxRGB(nBytesReceived, LTCDISPLAYWS28XX_COLOUR_INDEX_COLON);
						}

						return;
					}
					// ws28xx/rgb/message iii
					if (nCommandLength == (m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH + WS28XXRGBMESSAGE_LENGTH)) {
						if (memcmp(&m_pBuffer[m_nPathLength + WS28XX_LENGTH + WS28XXRGB_LENGTH], sWS28xxRgbMessage, WS28XXRGBMESSAGE_LENGTH) == 0) {
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

		DEBUG_PRINTF("*/ws28xx/rgb/[%d] -> %d %d %d", (int) tIndex, (int) nRed, (int) nGreen, (int) nBlue);
	} else {
		DEBUG_PUTS("Invalid ws28xx/rgb/*");
	}
}

void OSCServer::Print(void) {
	printf("OSC Server\n");
	printf(" Port : %d\n", m_nPortIncoming);
	printf(" Path : [%s]\n", m_aPath);
}
