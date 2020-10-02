/**
 * @file ltcparamsdump.cpp
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdio.h>

#include "ltcparams.h"
#include "ltcparamsconst.h"

void LtcParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcParamsConst::FILE_NAME);

	if (isMaskSet(LtcParamsMask::SOURCE)) {
		printf(" %s=%d [%s]\n", LtcParamsConst::SOURCE, m_tLtcParams.tSource, GetSourceType(static_cast<ltc::source>(m_tLtcParams.tSource)));
	}

	if (isMaskSet(LtcParamsMask::AUTO_START)) {
		printf(" %s=%d\n", LtcParamsConst::AUTO_START, m_tLtcParams.nAutoStart);
	}

	if (isMaskSet(LtcParamsMask::DISABLED_OUTPUTS)) {
		printf(" Disabled outputs %.2x:\n", m_tLtcParams.nDisabledOutputs);

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY)) {
			printf("  Display\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219)) {
			printf("  Max7219\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI)) {
			printf("  MIDI\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI)) {
			printf("  RtpMIDI\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET)) {
			printf("  Art-Net\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::TCNET)) {
			printf("  TCNet\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC)) {
			printf("  LTC\n");
		}
	}

	if (isMaskSet(LtcParamsMask::YEAR)) {
		printf(" %s=%d\n", LtcParamsConst::YEAR, m_tLtcParams.nYear);
	}

	if (isMaskSet(LtcParamsMask::MONTH)) {
		printf(" %s=%d\n", LtcParamsConst::MONTH, m_tLtcParams.nMonth);
	}

	if (isMaskSet(LtcParamsMask::DAY)) {
		printf(" %s=%d\n", LtcParamsConst::DAY, m_tLtcParams.nDay);
	}

	if (isMaskSet(LtcParamsMask::ENABLE_NTP)) {
		printf(" NTP is enabled\n");
	}

	if (isMaskSet(LtcParamsMask::FPS)) {
		printf(" %s=%d\n", LtcParamsConst::FPS, m_tLtcParams.nFps);
	}

	if (isMaskSet(LtcParamsMask::START_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::START_FRAME, m_tLtcParams.nStartFrame);
	}

	if (isMaskSet(LtcParamsMask::START_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::START_SECOND, m_tLtcParams.nStartSecond);
	}

	if (isMaskSet(LtcParamsMask::START_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::START_MINUTE, m_tLtcParams.nStartMinute);
	}

	if (isMaskSet(LtcParamsMask::START_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::START_HOUR, m_tLtcParams.nStartHour);
	}

	if (isMaskSet(LtcParamsMask::STOP_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_FRAME, m_tLtcParams.nStopFrame);
	}

	if (isMaskSet(LtcParamsMask::STOP_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_SECOND, m_tLtcParams.nStopSecond);
	}

	if (isMaskSet(LtcParamsMask::STOP_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_MINUTE, m_tLtcParams.nStopMinute);
	}

	if (isMaskSet(LtcParamsMask::STOP_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_HOUR, m_tLtcParams.nStopHour);
	}

	if (isMaskSet(LtcParamsMask::ALT_FUNCTION)) {
		printf(" %s=%d\n", LtcParamsConst::ALT_FUNCTION, m_tLtcParams.nAltFunction);
	}

	if (isMaskSet(LtcParamsMask::SKIP_SECONDS)) {
		printf(" %s=%d\n", LtcParamsConst::SKIP_SECONDS, m_tLtcParams.nSkipSeconds);
	}

#if 0
	if (isMaskSet(LtcParamsMask::SET_DATE)) {
		printf(" %s=%d\n", LtcParamsConst::SET_DATE, m_tLtcParams.nSetDate);
	}
#endif

	if (isMaskSet(LtcParamsMask::ENABLE_OSC)) {
		printf(" OSC is enabled\n");

		if (isMaskSet(LtcParamsMask::OSC_PORT)) {
			printf(" %s=%d\n", LtcParamsConst::OSC_PORT, m_tLtcParams.nOscPort);
		}
	}

	if (isMaskSet(LtcParamsMask::RGBLEDTYPE)) {
		if (m_tLtcParams.nRgbLedType == static_cast<uint8_t>(TLtcParamsRgbLedType::WS28XX)) {
			printf(" WS28xx is enabled\n");
		} else if (m_tLtcParams.nRgbLedType == static_cast<uint8_t>(TLtcParamsRgbLedType::RGBPANEL)) {
			printf(" RGB panel is enabled\n");
		} else {
			printf("nRgbLedType=%u\n", m_tLtcParams.nRgbLedType);
		}
	}
#endif
}
