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
	const bool isOn = !(Display::Get()->isSleep());
	const uint16_t nLength = static_cast<uint16_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"display\":%d}", isOn));
	return nLength;
}

uint16_t json_get_directory(char *pOutBuffer, const uint16_t nOutBufferSize) {
	const uint16_t nLength = static_cast<uint16_t>(snprintf(pOutBuffer, nOutBufferSize,
			"{\"files\":["
#if defined (NODE_ARTNET)
			"{\"name\":\"artnet.txt\",\"label\":\"Art-Net\"},"
#endif
#if defined (NODE_E131)
			"{\"name\":\"e131.txt\",\"label\":\"sACN E1.31\"},"
#endif
#if defined (NODE_OSC_CLIENT)
			"{\"name\":\"oscclnt.txt\",\"label\":\"OSC Client\"},"
#endif
#if defined (NODE_OSC_SERVER)
			"{\"name\":\"osc.txt\",\"label\":\"OSC Server\"},"
#endif
#if defined (NODE_LTC_SMPTE)
			"{\"name\":\"ltc.txt\",\"label\":\"LTC SMPTE\"},"
			"{\"name\":\"ldisplay.txt\",\"label\":\"Display\"},"
			"{\"name\":\"tcnet.txt\",\"label\":\"TCNet\"},"
			"{\"name\":\"gps.txt\",\"label\":\"GPS\"},"
#endif
#if defined(NODE_SHOWFILE)
			"{\"name\":\"show.txt\",\"label\":\"Showfile\"},"
#endif
#if defined(NODE_DDP_DISPLAY)
			"{\"name\":\"ddpdisp.txt\",\"label\":\"DDP Display\"},"
#endif
#if defined (OUTPUT_DMX_SEND)
			"{\"name\":\"params.txt\",\"label\":\"DMX Transmit\"},"
#endif
#if defined (OUTPUT_DMX_PIXEL)
			"{\"name\":\"devices.txt\",\"label\":\"DMX Pixel\"},"
#endif
#if defined (OUTPUT_DMX_MONITOR)
			"{\"name\":\"mon.txt\",\"label\":\"DMX Monitor\"},"
#endif
#if defined (OUTPUT_DMX_SERIAL)
			"{\"name\":\"serial.txt\",\"label\":\"DMX Serial\"},"
#endif
#if defined (OUTPUT_RGB_PANEL)
			"{\"name\":\"rgbpanel.txt\",\"label\":\"RGB panel\"},"
#endif
#if defined (RDM_RESPONDER)
			"{\"name\":\"sensors.txt\",\"label\":\"RDM Sensors\"},"
			"{\"name\":\"subdev.txt\",\"label\":\"RDM Sub devices\"},"
#endif
#if defined(DISPLAY_UDF)
			"{\"name\":\"display.txt\",\"label\":\"Display UDF\"},"
#endif
			"{\"name\":\"network.txt\",\"label\":\"Network\"},"
			"{\"name\":\"rconfig.txt\",\"label\":\"Remote configuration\"}"
			"]};"
			));
	return nLength;
}

}
