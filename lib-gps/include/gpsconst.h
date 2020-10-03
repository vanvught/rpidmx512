/**
 * @file gpsconst.h
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

#ifndef GPSCONST_H_
#define GPSCONST_H_

#include <stdint.h>

namespace gps {
namespace nmea{
static constexpr uint32_t MAX_SENTENCE_LENGTH = 82;	///< including the $ and <CR><LF>
static constexpr char START_DELIMITER = '$';		///< The start delimiter is normally '$' (ASCII 36)
}  // namespace nmea
namespace module {
static constexpr auto MAX_NAME_LENGTH = 8 + 1;  	// + '\0'
}  // namespace module
}  // namespace gps

enum class GPSModule {
	ATGM336H,
//	UBLOX,
//	ADAFRUIT,
	UNDEFINED
};

struct GPSConst {
	static const char MODULE[static_cast<unsigned>(GPSModule::UNDEFINED)][gps::module::MAX_NAME_LENGTH];

	static const char BAUD_115200[static_cast<unsigned>(GPSModule::UNDEFINED)][gps::nmea::MAX_SENTENCE_LENGTH];
};

#endif /* GPSCONST_H_ */
