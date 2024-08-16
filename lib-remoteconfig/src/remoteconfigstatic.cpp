/**
 * @file remoteconfigstatic.cpp
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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "remoteconfig.h"
#include "configstore.h"

#include "debug.h"

using namespace configstore;
using namespace remoteconfig;

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)

constexpr RemoteConfig::Txt RemoteConfig::s_TXT[] = {
		{ &RemoteConfig::HandleGetRconfigTxt,    &RemoteConfig::HandleSetRconfigTxt,    "rconfig.txt",  11},
		{ &RemoteConfig::HandleGetEnvTxt,        &RemoteConfig::HandleSetEnvTxt,        "env.txt",      7 },
		{ &RemoteConfig::HandleGetNetworkTxt,    &RemoteConfig::HandleSetNetworkTxt,    "network.txt",  11},
#if defined (DISPLAY_UDF)
		{ &RemoteConfig::HandleGetDisplayTxt,    &RemoteConfig::HandleSetDisplayTxt,    "display.txt",  11},
#endif
#if defined (NODE_ARTNET)
		{ &RemoteConfig::HandleGetArtnetTxt,     &RemoteConfig::HandleSetArtnetTxt,     "artnet.txt",   10},
#endif
#if defined (NODE_E131)
		{ &RemoteConfig::HandleGetE131Txt,       &RemoteConfig::HandleSetE131Txt,       "e131.txt",     8 },
#endif
#if defined (NODE_LTC_SMPTE)
		{ &RemoteConfig::HandleGetLtcTxt,        &RemoteConfig::HandleSetLtcTxt,        "ltc.txt",      7 },
		{ &RemoteConfig::HandleGetLdisplayTxt,   &RemoteConfig::HandleSetLdisplayTxt,   "ldisplay.txt", 12},
		{ &RemoteConfig::HandleGetTCNetTxt,      &RemoteConfig::HandleSetTCNetTxt,      "tcnet.txt",    9 },
		{ &RemoteConfig::HandleGetGpsTxt,        &RemoteConfig::HandleSetGpsTxt,        "gps.txt",      7 },
		{ &RemoteConfig::HandleGetLtcEtcTxt,     &RemoteConfig::HandleSetLtcEtcTxt,     "etc.txt",      7 },
#endif
#if defined (NODE_OSC_SERVER)
		{ &RemoteConfig::HandleGetOscTxt,        &RemoteConfig::HandleSetOscTxt,        "osc.txt",      7 },
#endif
#if defined (NODE_OSC_CLIENT)
		{ &RemoteConfig::HandleGetOscClntTxt,    &RemoteConfig::HandleSetOscClientTxt,  "oscclnt.txt",  11},
#endif
#if defined (NODE_SHOWFILE)
		{ &RemoteConfig::HandleGetShowTxt,       &RemoteConfig::HandleSetShowTxt,       "show.txt",     8 },
#endif
#if defined (NODE_NODE)
		{ &RemoteConfig::HandleGetNodeNodeTxt,   &RemoteConfig::HandleSetNodeNodeTxt,   "node.txt",     8 },
		{ &RemoteConfig::HandleGetNodeArtNetTxt, &RemoteConfig::HandleSetNodeArtNetTxt, "artnet.txt",   10},
		{ &RemoteConfig::HandleGetNodeE131Txt,   &RemoteConfig::HandleSetNodeE131Txt,   "e131.txt",     8 },
#endif
#if defined (RDM_RESPONDER)
		{ &RemoteConfig::HandleGetRdmDeviceTxt,  &RemoteConfig::HandleSetRdmDeviceTxt,  "rdm_device.txt", 14},
		{ &RemoteConfig::HandleGetRdmSensorsTxt, &RemoteConfig::HandleSetRdmSensorsTxt, "sensors.txt",    11},
# if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
		{ &RemoteConfig::HandleGetRdmSubdevTxt,  &RemoteConfig::HandleSetRdmSubdevTxt,  "subdev.txt",     10},
# endif
#endif
#if defined (OUTPUT_DMX_SEND)
		{ &RemoteConfig::HandleGetParamsTxt,     &RemoteConfig::HandleSetParamsTxt,     "params.txt",   10},
#endif
#if defined (OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_TLC59711)
		{ &RemoteConfig::HandleGetDevicesTxt,    &RemoteConfig::HandleSetDevicesTxt,    "devices.txt",  11},
#endif
#if defined (OUTPUT_DMX_MONITOR)
		{ &RemoteConfig::HandleGetMonTxt,        &RemoteConfig::HandleSetMonTxt,        "mon.txt",      7 },
#endif
#if defined (OUTPUT_DMX_SERIAL)
		{ &RemoteConfig::HandleGetSerialTxt,     &RemoteConfig::HandleSetSerialTxt,     "serial.txt",   10},
#endif
#if defined (OUTPUT_RGB_PANEL)
		{ &RemoteConfig::HandleGetRgbPanelTxt,   &RemoteConfig::HandleSetRgbPanelTxt,   "rgbpanel.txt", 12},
#endif
#if defined (OUTPUT_DMX_PCA9685)
		{ &RemoteConfig::HandleGetPca9685Txt,    &RemoteConfig::HandleSetPca9685Txt,    "pca9685.txt",  11},
#endif
#if defined(OUTPUT_DMX_STEPPER)
		{ &RemoteConfig::HandleGetSparkFunTxt,   &RemoteConfig::HandleSetSparkFunTxt,   "sparkfun.txt", 12},
		{ &RemoteConfig::HandleGetMotor0Txt,     &RemoteConfig::HandleSetMotor0Txt,     "motor0.txt",   10},
		{ &RemoteConfig::HandleGetMotor1Txt,     &RemoteConfig::HandleSetMotor1Txt,     "motor1.txt",   10},
		{ &RemoteConfig::HandleGetMotor2Txt,     &RemoteConfig::HandleSetMotor2Txt,     "motor2.txt",   10},
		{ &RemoteConfig::HandleGetMotor3Txt,     &RemoteConfig::HandleSetMotor3Txt,     "motor3.txt",   10},
		{ &RemoteConfig::HandleGetMotor4Txt,     &RemoteConfig::HandleSetMotor4Txt,     "motor4.txt",   10},
		{ &RemoteConfig::HandleGetMotor5Txt,     &RemoteConfig::HandleSetMotor5Txt,     "motor5.txt",   10},
		{ &RemoteConfig::HandleGetMotor6Txt,     &RemoteConfig::HandleSetMotor6Txt,     "motor6.txt",   10},
		{ &RemoteConfig::HandleGetMotor7Txt,     &RemoteConfig::HandleSetMotor7Txt,     "motor7.txt",   10}
#endif
};

int32_t RemoteConfig::GetIndex(const void *p, uint32_t& nLength) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nLength=%d", nLength);

#ifndef NDEBUG
	debug_dump(const_cast<void*>(p), 16);
#endif

	int32_t i;
	for (i = 0; i < static_cast<int32_t>(sizeof(s_TXT) / sizeof(s_TXT[0])); i++) {
		const auto *t = &s_TXT[i];
		assert(t->nFileNameLength == strlen(t->pFileName));
		if (memcmp(p, t->pFileName, std::min(static_cast<uint32_t>(t->nFileNameLength), nLength)) == 0) {
			nLength = t->nFileNameLength;
			DEBUG_EXIT;
			return i;
		}
	}

	DEBUG_EXIT
	return -1;
}

#endif
