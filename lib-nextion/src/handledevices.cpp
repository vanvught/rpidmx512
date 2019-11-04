/**
 * @file handledevices.cpp
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
#include <string.h>
#include <assert.h>

#include "nextion.h"

#include "ws28xxdmxparams.h"
#include "storews28xxdmx.h"

#include "tlc59711dmxparams.h"
#include "storetlc59711.h"

#include "devicesparamsconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

void Nextion::HandleDevicesGet(void) {
	DEBUG2_ENTRY

	bool bIsSetLedType = false;

	TLC59711DmxParams tlc5911params((TLC59711DmxParamsStore *) StoreTLC59711::Get());

#if !defined (PIXEL_MULTI)
	if (tlc5911params.Load()) {
		if ((bIsSetLedType = tlc5911params.IsSetLedType()) == true) {
			SetText("p_type", tlc5911params.GetLedTypeString(tlc5911params.GetLedType()));
			SetValue("p_count", static_cast<uint32_t>(tlc5911params.GetLedCount()));
		}
	}
#endif

	if (!bIsSetLedType) {
		WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) StoreWS28xxDmx::Get());

		if (ws28xxparms.Load()) {
			SetText("p_type", ws28xxparms.GetLedTypeString(ws28xxparms.GetLedType()));
			SetValue("p_count", static_cast<uint32_t>(ws28xxparms.GetLedCount()));

			SetValue("p_grp", static_cast<uint32_t>(ws28xxparms.IsLedGrouping()));
			if (ws28xxparms.IsLedGrouping()) {
				SetValue("p_grpcount", static_cast<uint32_t>(ws28xxparms.GetLedGroupCount()));
			}

			SetValue("p_ports", static_cast<uint32_t>(ws28xxparms.GetActivePorts()));
		}
	}

	DEBUG2_EXIT
}

void Nextion::HandleDevicesSave(void) {
	DEBUG2_ENTRY

	uint32_t nValue;
	char pValue[16];
	uint8_t aBuffer[128];

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, aBuffer, static_cast<uint32_t>(sizeof aBuffer));

	uint32_t nLength = (sizeof pValue) - 1;

	if (GetText("p_type", pValue, nLength)) {
#if !defined (PIXEL_MULTI)
		const TTLC59711Type tlc5911Type = TLC59711DmxParams::GetLedTypeString(pValue);
		if (tlc5911Type != TTLC59711_TYPE_UNDEFINED) {
			builder.Add(DevicesParamsConst::LED_TYPE, static_cast<uint32_t>(tlc5911Type), true);
		} else {
#endif
			const TWS28XXType ws28xxType = WS28xxDmxParams::GetLedTypeString(pValue);
			if (ws28xxType != WS28XX_UNDEFINED) {
				builder.Add(DevicesParamsConst::LED_TYPE, static_cast<uint32_t>(ws28xxType), true);
			}
#if !defined (PIXEL_MULTI)
		}
#endif
	}

	if (GetValue("p_count", nValue)) {
		builder.Add(DevicesParamsConst::LED_COUNT, nValue, true);
	}

#if defined (PIXEL)
	if (GetValue("p_grp", nValue)) {
		if (static_cast<bool>(nValue)) {
			builder.Add(DevicesParamsConst::LED_GROUPING, true, true);
			if (GetValue("p_grpcount", nValue)) {
				builder.Add(DevicesParamsConst::LED_GROUP_COUNT, nValue, true);
			}
		}
	}
#endif

#if defined (PIXEL_MULTI)
	if (GetValue("p_ports", nValue)) {
		builder.Add(DevicesParamsConst::ACTIVE_OUT, nValue, true);
	}
#endif

	WS28xxDmxParams ws28xxDmxParams(static_cast<WS28xxDmxParamsStore *>(StoreWS28xxDmx::Get()));

	ws28xxDmxParams.Load(reinterpret_cast<const char *>(aBuffer), builder.GetSize());

	DEBUG2_EXIT
}
