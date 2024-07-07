/**
 * @file remoteconfigjson.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "remoteconfig.h"
#include "hardware.h"
#include "network.h"
#include "display.h"
#include "firmwareversion.h"

namespace remoteconfig {

uint32_t json_get_list(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
						"{\"list\":{\"ip\":\"" IPSTR "\",\"name\":\"%s\",\"node\":{\"type\":\"%s\",\"port\":{\"type\":\"%s\",\"count\":%d}}}}",
						IP2STR(Network::Get()->GetIp()),
						RemoteConfig::Get()->GetDisplayName(),
						RemoteConfig::Get()->GetStringNode(),
						RemoteConfig::Get()->GetStringOutput(),
						RemoteConfig::Get()->GetOutputs()));

	return nLength;
}

uint32_t json_get_version(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto *pVersion = FirmwareVersion::Get()->GetVersion();
	uint8_t nHwTextLength;

	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
					"{\"version\":\"%.*s\",\"board\":\"%s\",\"build\":{\"date\":\"%.*s\",\"time\":\"%.*s\"}}",
					firmwareversion::length::SOFTWARE_VERSION, pVersion->SoftwareVersion,
					Hardware::Get()->GetBoardName(nHwTextLength),
					firmwareversion::length::GCC_DATE, pVersion->BuildDate,
					firmwareversion::length::GCC_TIME, pVersion->BuildTime));
	return nLength;
}

uint32_t json_get_uptime(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nUptime = Hardware::Get()->GetUpTime();
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"uptime\":%u}\n", static_cast<unsigned int>(nUptime)));
	return nLength;
}

uint32_t json_get_display(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const bool isOn = !(Display::Get()->isSleep());
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"display\":%d}", isOn));
	return nLength;
}

uint32_t json_get_directory(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
			"{\"files\":{"
#if defined (NODE_ARTNET)
			"\"artnet.txt\":\"Art-Net\","
#endif
#if defined (NODE_E131)
			"\"e131.txt\":\"sACN E1.31\","
#endif
#if defined (NODE_OSC_CLIENT)
			"\"oscclnt.txt\":\"OSC Client\","
#endif
#if defined (NODE_OSC_SERVER)
			"\"osc.txt\":\"OSC Server\","
#endif
#if defined (NODE_LTC_SMPTE)
			"\"ltc.txt\":\"LTC SMPTE\","
			"\"ldisplay.txt\":\"Display\","
			"\"tcnet.txt\":\"TCNet\","
			"\"gps.txt\":\"GPS\","
			"\"etc.txt\":\"ETC gateway\","
#endif
#if defined(NODE_SHOWFILE)
			"\"show.txt\":\"Showfile\","
#endif
#if defined(NODE_NODE)
			"\"node.txt\":\"Node\","
			"\"artnet.txt\":\"Art-Net\","
			"\"e131.txt\":\"sACN E1.31\","
#endif
#if defined (OUTPUT_DMX_SEND)
			"\"params.txt\":\"DMX Transmit\","
#endif
#if defined (OUTPUT_DMX_PIXEL)
			"\"devices.txt\":\"DMX Pixel\","
#endif
#if defined (OUTPUT_DMX_TLC59711)
			"\"devices.txt\":\"DMX TLC59711\","
#endif
#if defined (OUTPUT_DMX_PCA9685)
			"\"pca9685.txt\":\"DMX PCA9685\","
#endif
#if defined (OUTPUT_DMX_MONITOR)
			"\"mon.txt\":\"DMX Monitor\","
#endif
#if defined (OUTPUT_DMX_SERIAL)
			"\"serial.txt\":\"DMX Serial\","
#endif
#if defined (OUTPUT_RGB_PANEL)
			"\"rgbpanel.txt\":\"RGB panel\","
#endif
#if defined (OUTPUT_DMX_STEPPER)
			"\"sparkfun.txt\":\"SparkFun\","
			"\"motor0.txt\":\"Stepper 1\","
			"\"motor1.txt\":\"Stepper 2\","
			"\"motor2.txt\":\"Stepper 3\","
			"\"motor3.txt\":\"Stepper 4\","
			"\"motor4.txt\":\"Stepper 5\","
			"\"motor5.txt\":\"Stepper 6\","
			"\"motor6.txt\":\"Stepper 7\","
			"\"motor7.txt\":\"Stepper 8\","
#endif
#if defined (RDM_RESPONDER)
			"\"rdm_device.txt\":\"RDM Device\","
			"\"sensors.txt\":\"RDM Sensors\","
#endif
#if defined(DISPLAY_UDF)
			"\"display.txt\":\"Display UDF\","
#endif
			"\"network.txt\":\"Network\","
			"\"env.txt\":\"Environment\","
			"\"rconfig.txt\":\"Remote configuration\""
			"}}"
			));
	return nLength;
}
}  // namespace remoteconfig
