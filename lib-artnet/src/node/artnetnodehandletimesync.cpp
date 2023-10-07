/**
 * @file artnetnodehandletimesync.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetnode.h"

#include "hardware.h"

#include "debug.h"

void ArtNetNode::HandleTimeSync() {
	const auto *const pArtTimeSync = reinterpret_cast<artnet::ArtTimeSync *>(m_pReceiveBuffer);
	struct tm tmTime;

	tmTime.tm_sec = pArtTimeSync->tm_sec;
	tmTime.tm_min = pArtTimeSync->tm_min;
	tmTime.tm_hour = pArtTimeSync->tm_hour;
	tmTime.tm_mday = pArtTimeSync->tm_mday;
	tmTime.tm_mon = pArtTimeSync->tm_mon;
	tmTime.tm_year = ((pArtTimeSync->tm_year_hi) << 8) + pArtTimeSync->tm_year_lo;

	Hardware::Get()->SetTime(&tmTime);

	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", 1900 + tmTime.tm_year, 1 + tmTime.tm_mon, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
}
