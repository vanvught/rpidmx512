/**
 * @file handlemainpage.cpp
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
#include <stdio.h>

#include "nextion.h"

#include "firmwareversion.h"
#include "hardware.h"

#include "debug.h"

void Nextion::HandleMain(void) {
	DEBUG1_ENTRY

	{
		char componentText[] = "title.txt=\"Art-Net 4 Node Pixel Controller\""; //FIXME title.txt
		SendCommand(componentText);
	}

	SetText("version", FirmwareVersion::Get()->GetPrint());
	SetValue("uptime", static_cast<uint32_t>(Hardware::Get()->GetUpTime()));

#ifndef NDEBUG
	uint32_t nValue;
	bool bSucces = GetValue("uptime", nValue);

	DEBUG_PRINTF("%d:uptime=%d", bSucces, nValue);
#endif
	DEBUG1_EXIT
}
