/**
 * @file remoteconfig.cpp;
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

#include <cstdint>
#include <cstdio>

#include "hardware.h"
#include "display.h"
#include "firmwareversion.h"
#include "network.h"
#include "remoteconfig.h"

namespace remoteconfig {

uint16_t json_get_list(char *pOutBuffer, const uint16_t nOutBufferSize) {
	const uint16_t nLength = static_cast<uint16_t>(snprintf(pOutBuffer, nOutBufferSize,
						"{\"list\":{\"ip\":\"" IPSTR "\",\"name\":\"%s\",\"node\":{\"type\":\"%s\",\"port\":{\"type\":\"%s\",\"count\":%d}}}}",
						IP2STR(Network::Get()->GetIp()),
						RemoteConfig::Get()->GetDisplayName(),
						RemoteConfig::Get()->GetStringNode(),
						RemoteConfig::Get()->GetStringMode(),
						RemoteConfig::Get()->GetOutputs()));

	return nLength;
}

uint16_t json_get_version(char *pOutBuffer, const uint16_t nOutBufferSize) {
	const auto *pVersion = FirmwareVersion::Get()->GetVersion();
	uint8_t nHwTextLength;

	const uint16_t nLength = static_cast<uint16_t>(snprintf(pOutBuffer, nOutBufferSize,
					"{\"version\":\"%.*s\",\"board\":\"%s\",\"build\":{\"date\":\"%.*s\",\"time\":\"%.*s\"}}",
					firmwareversion::length::SOFTWARE_VERSION, pVersion->SoftwareVersion,
					Hardware::Get()->GetBoardName(nHwTextLength),
					firmwareversion::length::GCC_DATE, pVersion->BuildDate,
					firmwareversion::length::GCC_TIME, pVersion->BuildTime));
	return nLength;
}

uint16_t json_get_uptime(char *pOutBuffer, const uint16_t nOutBufferSize) {
	const auto nUptime = Hardware::Get()->GetUpTime();
	const uint16_t nLength = static_cast<uint16_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"uptime\":%u}\n", nUptime));
	return nLength;
}

uint16_t json_get_display(char *pOutBuffer, const uint16_t nOutBufferSize) {
	if (!Display::Get()) return 0;
	const bool isOn = !(Display::Get()->isSleep());
	const uint16_t nLength = static_cast<uint16_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"display\":%s}", isOn ? "true" : "false"));
	return nLength;
}

}
