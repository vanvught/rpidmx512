/**
 * @file gpsconst.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "gpsconst.h"

using namespace gps;

const char GPSConst::MODULE[static_cast<unsigned>(GPSModule::UNDEFINED)][module::MAX_NAME_LENGTH] = {
		"ATGM336H",
		"ublox",
		"Adafruit"
};

const char GPSConst::BAUD_REQUEST[static_cast<unsigned>(GPSModule::UNDEFINED)][nmea::MAX_SENTENCE_LENGTH] = {
		"$..\r\n",
		"$..\r\n",
		"$..\r\n"
};

const char GPSConst::BAUD_RESPONSE_OK[static_cast<unsigned>(GPSModule::UNDEFINED)][nmea::MAX_SENTENCE_LENGTH] = {
		"$..\r\n",
		"$..\r\n",
		"$..\r\n"
};
