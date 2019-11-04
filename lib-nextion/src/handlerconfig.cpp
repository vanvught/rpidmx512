/**
 * @file handlerconfig.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "nextion.h"

#include "remoteconfigparams.h"
#include "remoteconfigconst.h"
#include "storeremoteconfig.h"
#include "propertiesbuilder.h"

#include "debug.h"

void Nextion::HandleRconfigGet(void) {
	DEBUG2_ENTRY

	SetValue("r_disable", static_cast<uint32_t>(RemoteConfig::Get()->GetDisable()));
	SetValue("r_write", static_cast<uint32_t>(RemoteConfig::Get()->GetDisableWrite()));
	SetValue("r_reboot", static_cast<uint32_t>(RemoteConfig::Get()->GetEnableReboot()));
	SetValue("r_uptime", static_cast<uint32_t>(RemoteConfig::Get()->GetEnableUptime()));

	DEBUG2_EXIT
}

void Nextion::HandleRconfigSave(void) {
	DEBUG2_ENTRY

	uint32_t nValue;
	uint8_t aBuffer[80];

	PropertiesBuilder builder(RemoteConfigConst::PARAMS_FILE_NAME, aBuffer, static_cast<uint32_t>(sizeof aBuffer));

	if (GetValue("r_disable", nValue)) {
		RemoteConfig::Get()->SetDisable(static_cast<bool>(nValue));
		builder.Add(RemoteConfigConst::PARAMS_DISABLE, true, static_cast<bool>(nValue));
	}

	if (GetValue("r_write", nValue)) {
		RemoteConfig::Get()->SetDisableWrite(static_cast<bool>(nValue));
		builder.Add(RemoteConfigConst::PARAMS_DISABLE_WRITE, true, static_cast<bool>(nValue));
	}

	if (GetValue("r_reboot", nValue)) {
		RemoteConfig::Get()->SetEnableReboot(static_cast<bool>(nValue));
		builder.Add(RemoteConfigConst::PARAMS_ENABLE_REBOOT, true, static_cast<bool>(nValue));
	}

	if (GetValue("r_uptime", nValue)) {
		RemoteConfig::Get()->SetEnableUptime(static_cast<bool>(nValue));
		builder.Add(RemoteConfigConst::PARAMS_ENABLE_UPTIME, true, static_cast<bool>(nValue));
	}

	RemoteConfigParams remoteConfigParams(static_cast<RemoteConfigParamsStore *>(StoreRemoteConfig::Get()));

	remoteConfigParams.Load(reinterpret_cast<const char *>(aBuffer), builder.GetSize());

	DEBUG2_EXIT
}
