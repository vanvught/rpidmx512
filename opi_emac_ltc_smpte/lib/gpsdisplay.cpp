/**
 * @file gpsdisplay.cpp
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

#include <cassert>

#include "gps.h"

#include "display.h"

void GPS::Display(GPSStatus status) {
	Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 7U), 3);
	Display::Get()->PutString("GPS ");

	switch (status) {
	case GPSStatus::IDLE:
		Display::Get()->PutString("[I]");
		break;
	case GPSStatus::WARNING:
		Display::Get()->PutString("[W]");
		break;
	case GPSStatus::VALID:
		Display::Get()->PutString("[V]");
		break;
	case GPSStatus::UNDEFINED:
		Display::Get()->PutString("[U]");
		break;
	default:
		assert(0);
		break;
	}
}
