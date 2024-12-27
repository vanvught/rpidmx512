/**
 * @file oscserver.cpp
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

#if defined (DEBUG_LTCOSCSERVER)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ltcoscserver.h"

#if !defined (LTC_NO_DISPLAY_RGB)
# include "ltcdisplayrgb.h"
#endif

#include "arm/ltcmidisystemrealtime.h"
#include "arm/ltcgenerator.h"
#include "arm/ltcoutputs.h"
#include "arm/systimereader.h"

#include "tcnetdisplay.h"
#include "tcnet.h"
#include "midi.h"
#include "osc.h"
#include "oscsimplemessage.h"

#include "network.h"

#include "debug.h"

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
static constexpr uint32_t START = sizeof(cmd::START) - 1;
static constexpr uint32_t STOP = sizeof(cmd::STOP) - 1;
static constexpr uint32_t RESUME = sizeof(cmd::RESUME) - 1;
static constexpr uint32_t RATE = sizeof(cmd::RATE) - 1;
static constexpr uint32_t SET = sizeof(cmd::SET) - 1;
static constexpr uint32_t GOTO = sizeof(cmd::GOTO) - 1;
static constexpr uint32_t DIRECTION = sizeof(cmd::DIRECTION) - 1;
static constexpr uint32_t PITCH = sizeof(cmd::PITCH) - 1;
static constexpr uint32_t FORWARD = sizeof(cmd::FORWARD) - 1;
static constexpr uint32_t BACKWARD = sizeof(cmd::BACKWARD) - 1;
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
} // namespace tcnet

namespace midi {
namespace cmd {
static constexpr char PATH[] = "midi/";
static constexpr char START[] = "start";
static constexpr char STOP[] = "stop";
static constexpr char CONTINUE[] = "continue";
static constexpr char BPM[] = "bpm";
} // namespace cmd
namespace length {
static constexpr uint32_t PATH = sizeof(cmd::PATH) - 1;
static constexpr uint32_t START = sizeof(cmd::START) - 1;
static constexpr uint32_t STOP = sizeof(cmd::STOP) - 1;
static constexpr uint32_t RESUME = sizeof(cmd::CONTINUE) - 1;
static constexpr uint32_t BPM = sizeof(cmd::BPM) - 1;
} // namespace length
} // namespace midi

namespace ws28xx {
namespace cmd {
static constexpr char PATH[] = "ws28xx/";
static constexpr char MASTER[] = "master";
static constexpr char MESSAGE[] = "message";
static constexpr char INFO[] = "info";
}  // namespace cmd
namespace length {
static constexpr uint32_t PATH = sizeof(cmd::PATH) - 1;
static constexpr uint32_t MASTER = sizeof(cmd::MASTER) - 1;
static constexpr uint32_t MESSAGE = sizeof(cmd::MESSAGE) - 1;
static constexpr uint32_t INFO = sizeof(cmd::INFO) - 1;
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
static constexpr uint32_t PATH = sizeof(cmd::PATH) - 1;
static constexpr uint32_t TIME = sizeof(cmd::TIME) - 1;
static constexpr uint32_t COLON = sizeof(cmd::COLON) - 1;
static constexpr uint32_t MESSAGE = sizeof(cmd::MESSAGE) - 1;
static constexpr uint32_t FPS = sizeof(cmd::FPS) - 1;
static constexpr uint32_t INFO = sizeof(cmd::INFO) - 1;
} // namespace length
} // namespace rgb
} // namespace ws28xx

// "hh/mm/ss/ff" -> length = 11
static constexpr uint32_t VALUE_LENGTH = 11;
static constexpr uint32_t FPS_VALUE_LENGTH = 2;

void LtcOscServer::Input(const uint8_t *pData, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	if (nSize <= 4) {
		return;
	}

	const auto *pDataChar = reinterpret_cast<const char *>(pData);

	if (osc::is_match(pDataChar, m_aPath)) {
		const auto nCommandLength = strlen(pDataChar);

		DEBUG_PRINTF("[%s]:%d %d:|%s|", pDataChar, static_cast<int>(nCommandLength), m_nPathLength, &pDataChar[m_nPathLength]);

		// */pitch f
		if (memcmp(&pDataChar[m_nPathLength], cmd::PITCH, length::PITCH) == 0) {
			OscSimpleMessage Msg(pData, nSize);

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
		if (memcmp(&pDataChar[m_nPathLength], cmd::START, length::START) == 0) {
			if ((nCommandLength == (m_nPathLength + length::START)) ) {

				LtcGenerator::Get()->ActionStart();
				SystimeReader::Get()->ActionStart();

				DEBUG_PUTS("ActionStart");
			} else if ((nCommandLength == (m_nPathLength + length::START + 1 + VALUE_LENGTH))) {
				if (pDataChar[m_nPathLength + length::START] == '/') {
					const auto nOffset = m_nPathLength + length::START + 1;

					char timeCode[VALUE_LENGTH];
					memcpy(timeCode, &pDataChar[nOffset], VALUE_LENGTH);

					timeCode[2] = ':';
					timeCode[5] = ':';
					timeCode[8] = ':';

					LtcGenerator::Get()->ActionSetStart(timeCode);
					LtcGenerator::Get()->ActionStop();
					LtcGenerator::Get()->ActionStart();

					DEBUG_PRINTF("%.*s", VALUE_LENGTH, timeCode);
				}
			} else if ((nCommandLength == (m_nPathLength + length::START + length::SET + VALUE_LENGTH))) {
				if (memcmp(&pDataChar[m_nPathLength + length::START], cmd::SET, length::SET) == 0) {
					const auto nOffset = m_nPathLength + length::START + length::SET;

					char timeCode[VALUE_LENGTH];
					memcpy(timeCode, &pDataChar[nOffset], VALUE_LENGTH);

					timeCode[2] = ':';
					timeCode[5] = ':';
					timeCode[8] = ':';

					LtcGenerator::Get()->ActionSetStart(timeCode);

					DEBUG_PRINTF("%.*s", VALUE_LENGTH, timeCode);
				}
			}
			return;
		}
		// */stop*
		if (memcmp(&pDataChar[m_nPathLength], cmd::STOP, length::STOP) == 0) {
			if ((nCommandLength == (m_nPathLength + length::STOP))) {

				LtcGenerator::Get()->ActionStop();
				SystimeReader::Get()->ActionStop();

				DEBUG_PUTS("ActionStop");
			} else if ((nCommandLength == (m_nPathLength + length::STOP + length::SET + VALUE_LENGTH))) {
				if (memcmp(&pDataChar[m_nPathLength + length::STOP], cmd::SET, length::SET) == 0) {
					const auto nOffset = m_nPathLength + length::STOP + length::SET;

					char timeCode[VALUE_LENGTH];
					memcpy(timeCode, &pDataChar[nOffset], VALUE_LENGTH);

					timeCode[2] = ':';
					timeCode[5] = ':';
					timeCode[8] = ':';

					LtcGenerator::Get()->ActionSetStop(timeCode);

					DEBUG_PRINTF("%.*s", VALUE_LENGTH, timeCode);
				}
			}

			return;
		}
		// */forward i
		if (memcmp(&pDataChar[m_nPathLength], cmd::FORWARD, length::FORWARD) == 0) {
			OscSimpleMessage Msg(pData, nSize);

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
		if (memcmp(&pDataChar[m_nPathLength], cmd::BACKWARD, length::BACKWARD) == 0) {
			OscSimpleMessage Msg(pData, nSize);

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
			if (memcmp(&pDataChar[m_nPathLength + length::RATE], cmd::SET, length::SET) == 0) {
				const auto nOffset = m_nPathLength + length::RATE + length::SET;

				LtcGenerator::Get()->ActionSetRate(&pDataChar[nOffset]);

				DEBUG_PUTS(&pDataChar[nOffset]);
				return;
			}
		}
		// */resume
		if ( (nCommandLength == (m_nPathLength + length::RESUME)) && (memcmp(&pDataChar[m_nPathLength], cmd::RESUME, length::RESUME) == 0)) {
			LtcGenerator::Get()->ActionResume();

			DEBUG_PUTS("ActionResume");
			return;
		}
		// */goto/*
		if ((nCommandLength == (m_nPathLength + length::GOTO + 1 + VALUE_LENGTH)) && (memcmp(&pDataChar[m_nPathLength], cmd::GOTO, length::GOTO) == 0)) {
			if (pDataChar[m_nPathLength + length::GOTO] == '/') {
				const auto nOffset = m_nPathLength + length::GOTO + 1;

				char timeCode[VALUE_LENGTH];
				memcpy(timeCode, &pDataChar[nOffset], VALUE_LENGTH);

				timeCode[2] = ':';
				timeCode[5] = ':';
				timeCode[8] = ':';

				LtcGenerator::Get()->ActionGoto(timeCode);

				DEBUG_PUTS(timeCode);
				return;
			}
		}
		// */direction/*
		if ((nCommandLength <= (m_nPathLength + length::DIRECTION + 1 + 8)) && (memcmp(&pDataChar[m_nPathLength], cmd::DIRECTION, length::DIRECTION) == 0)) {
			if (pDataChar[m_nPathLength + length::DIRECTION] == '/') {
				const uint32_t nOffset = m_nPathLength + length::DIRECTION + 1;
				LtcGenerator::Get()->ActionSetDirection(&pDataChar[nOffset]);

				DEBUG_PUTS(&pDataChar[nOffset]);
				return;
			}
		}

		// */tcnet/
		if (memcmp(&pDataChar[m_nPathLength], tcnet::cmd::PATH, tcnet::length::PATH) == 0) {
			// layer/?
			if (nCommandLength == (m_nPathLength + tcnet::length::PATH + tcnet::length::LAYER + 1)) {
				if (memcmp(&pDataChar[m_nPathLength + tcnet::length::PATH], tcnet::cmd::LAYER, tcnet::length::LAYER) == 0) {
					const auto nOffset = m_nPathLength + tcnet::length::PATH + tcnet::length::LAYER;
					const auto tLayer = TCNet::GetLayer(pDataChar[nOffset]);

					TCNet::Get()->SetLayer(tLayer);
					tcnet::display::show();

					DEBUG_PRINTF("*/tcnet/layer/%c -> %d", pDataChar[nOffset], static_cast<int>(tLayer));
					return;
				}
			}
			// type i
			if (nCommandLength == (m_nPathLength + tcnet::length::PATH + tcnet::length::TYPE)) {
				if (memcmp(&pDataChar[m_nPathLength + tcnet::length::PATH], tcnet::cmd::TYPE, tcnet::length::TYPE) == 0) {
					OscSimpleMessage Msg(pData, nSize);

					if (Msg.GetType(0) != osc::type::INT32) {
						return;
					}

					const int nValue = Msg.GetInt(0);

					switch (nValue) {
					case 24:
						TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_FILM);
						tcnet::display::show();
						break;
					case 25:
						TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_EBU_25FPS);
						tcnet::display::show();
						break;
					case 29:
						TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_DF);
						tcnet::display::show();
						break;
					case 30:
						TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS);
						tcnet::display::show();
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
				if (memcmp(&pDataChar[m_nPathLength + tcnet::length::PATH], tcnet::cmd::TYPE, tcnet::length::TIMECODE) == 0) {
					OscSimpleMessage Msg(pData, nSize);

					if (Msg.GetType(0) != osc::type::INT32) {
						return;
					}

					const auto nValue = Msg.GetInt(0);
					const auto bUseTimeCode = (nValue > 0);

					TCNet::Get()->SetUseTimeCode(bUseTimeCode);
					tcnet::display::show();

					DEBUG_PRINTF("*/tcnet/timecode -> %d", static_cast<int>(bUseTimeCode));
					return;
				}
			}
		}

		// */midi/
		if (memcmp(&pDataChar[m_nPathLength], midi::cmd::PATH, midi::length::PATH) == 0) {
			// */start
			if ((nCommandLength == (m_nPathLength + midi::length::PATH + midi::length::START)) && (memcmp(&pDataChar[m_nPathLength + midi::length::PATH], midi::cmd::START, midi::length::START) == 0)) {
				LtcMidiSystemRealtime::Get()->SendStart();
				DEBUG_PUTS("MIDI Start");
				return;
			}
			// */stop
			if ((nCommandLength == (m_nPathLength + midi::length::PATH + midi::length::STOP)) && (memcmp(&pDataChar[m_nPathLength + midi::length::PATH], midi::cmd::STOP, midi::length::STOP) == 0)) {
				LtcMidiSystemRealtime::Get()->SendStop();
				DEBUG_PUTS("MIDI Stop");
				return;
			}
			// */continue
			if ((nCommandLength == (m_nPathLength + midi::length::PATH + midi::length::RESUME)) && (memcmp(&pDataChar[m_nPathLength + midi::length::PATH], midi::cmd::CONTINUE, midi::length::RESUME) == 0)) {
				LtcMidiSystemRealtime::Get()->SendContinue();
				DEBUG_PUTS("MIDI Continue");
				return;
			}
			// */bpm i or */bpm f
			if ((nCommandLength == (m_nPathLength + midi::length::PATH + midi::length::BPM)) && (memcmp(&pDataChar[m_nPathLength + midi::length::PATH], midi::cmd::BPM, midi::length::BPM) == 0)) {
				uint32_t nBPM = 0;

				OscSimpleMessage Msg(pData, nSize);

				if (Msg.GetType(0) == osc::type::FLOAT) {
					const auto fValue = Msg.GetFloat(0);
					if (fValue > 0) {
						nBPM = static_cast<uint32_t>(fValue);
					}
				} else if (Msg.GetType(0) == osc::type::INT32) {
					const auto nValue = Msg.GetInt(0);
					if (nValue > 0) {
						nBPM = static_cast<uint32_t>(nValue);
					}
				}

				LtcMidiSystemRealtime::Get()->SetBPM(nBPM);
				LtcOutputs::Get()->ShowBPM(nBPM);
				
				DEBUG_PRINTF("MIDI BPM: %u", nBPM);
				return;
			}
		}
#if !defined(LTC_NO_DISPLAY_RGB)
		// */ws28xx/
		if (memcmp(&pDataChar[m_nPathLength], ws28xx::cmd::PATH, ws28xx::length::PATH) == 0) {
			// ws28xx/master i
			if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::length::MASTER)) {
				if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH], ws28xx::cmd::MASTER, ws28xx::length::MASTER) == 0) {
					OscSimpleMessage Msg(pData, nSize);

					if (Msg.GetType(0) != osc::type::INT32) {
						return;
					}

					const auto nValue = static_cast<uint8_t>(Msg.GetInt(0));

					LtcDisplayRgb::Get()->SetMaster(nValue);

					DEBUG_PRINTF("*/ws28xx/master -> %d", static_cast<int>(static_cast<uint8_t>(nValue)));
					return;
				}
			}
			// ws28xx/message string
			if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::length::MESSAGE)) {
				if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH], ws28xx::cmd::MESSAGE, ws28xx::length::MESSAGE) == 0) {
					OscSimpleMessage Msg(pData, nSize);

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
				if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH], ws28xx::cmd::INFO, ws28xx::length::INFO) == 0) {
					OscSimpleMessage Msg(pData, nSize);

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
				if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH], ws28xx::rgb::cmd::PATH, ws28xx::rgb::length::PATH) == 0) {
					// ws28xx/rgb/time iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::TIME)) {
						if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::TIME, ws28xx::rgb::length::TIME) == 0) {
							SetWS28xxRGB(pData, nSize, ltcdisplayrgb::ColourIndex::TIME);
							return;
						}
					}
					// ws28xx/rgb/colon iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::COLON)) {
						if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::COLON, ws28xx::rgb::length::COLON) == 0) {
							SetWS28xxRGB(pData, nSize, ltcdisplayrgb::ColourIndex::COLON);
							return;
						}
					}
					// ws28xx/rgb/message iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::MESSAGE)) {
						if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::MESSAGE, ws28xx::rgb::length::MESSAGE) == 0) {
							SetWS28xxRGB(pData ,nSize, ltcdisplayrgb::ColourIndex::MESSAGE);
							return;
						}
					}
					// ws28xx/rgb/fps iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::FPS)) {
						if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::FPS, ws28xx::rgb::length::FPS) == 0) {
							SetWS28xxRGB(pData, nSize, ltcdisplayrgb::ColourIndex::FPS);
							return;
						}
					}
					// ws28xx/rgb/info iii
					if (nCommandLength == (m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH + ws28xx::rgb::length::INFO)) {
						if (memcmp(&pDataChar[m_nPathLength + ws28xx::length::PATH + ws28xx::rgb::length::PATH], ws28xx::rgb::cmd::INFO, ws28xx::rgb::length::INFO) == 0) {
							SetWS28xxRGB(pData, nSize, ltcdisplayrgb::ColourIndex::INFO);
							return;
						}
					}
					return;
				}
			}
		}
#endif
	}
}


#if !defined(LTC_NO_DISPLAY_RGB)
void LtcOscServer::SetWS28xxRGB(const uint8_t *pData, uint32_t nSize, ltcdisplayrgb::ColourIndex tIndex) {
	OscSimpleMessage Msg(pData, nSize);

	if (Msg.GetArgc() == 3) {
		const auto nRed = static_cast<uint8_t>(Msg.GetInt(0));
		const auto nGreen = static_cast<uint8_t>(Msg.GetInt(1));
		const auto nBlue = static_cast<uint8_t>(Msg.GetInt(2));

		LtcDisplayRgb::Get()->SetRGB(nRed, nGreen, nBlue, tIndex);

		DEBUG_PRINTF("*/ws28xx/rgb/[%d] -> %d %d %d", static_cast<int>(tIndex), static_cast<int>(nRed), static_cast<int>(nGreen), static_cast<int>(nBlue));
	} else {
		DEBUG_PUTS("Invalid ws28xx/rgb/*");
	}
}
#endif
