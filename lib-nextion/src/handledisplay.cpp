/**
 * @file handledisplay.cpp
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

#include "display.h"
#include "displayudfparams.h"
#include "displayudfparamsconst.h"
#include "storedisplayudf.h"
#include "propertiesbuilder.h"

#include "debug.h"

void Nextion::HandleDisplayGet(void) {
	DEBUG2_ENTRY

	SetValue("d_timeout", static_cast<uint32_t>(Display::Get()->GetSleepTimeout()));

	DEBUG2_EXIT
}

void Nextion::HandleDisplaySave(void) {
	DEBUG2_ENTRY

	uint32_t nValue;
	uint8_t aBuffer[32];

	PropertiesBuilder builder(DisplayUdfParamsConst::FILE_NAME, aBuffer, static_cast<uint32_t>(sizeof aBuffer));

	if (GetValue("d_timeout", nValue)) {
		Display::Get()->SetSleepTimeout(nValue);
		builder.Add(DisplayUdfParamsConst::SLEEP_TIMEOUT, nValue, true);
	}

	DisplayUdfParams displayUdfParams(static_cast<DisplayUdfParamsStore *>(StoreDisplayUdf::Get()));

	displayUdfParams.Load(reinterpret_cast<const char *>(aBuffer), builder.GetSize());

	DEBUG2_EXIT
}
