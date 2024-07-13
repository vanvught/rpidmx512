/**
 * @file artnettimecode.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@gd332-dmx.org
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

#ifndef ARTNETTIMECODE_H_
#define ARTNETTIMECODE_H_

#include <cstdint>

namespace artnet {
struct TimeCode {
	uint8_t Frames;			///< Frames time. 0 – 29 depending on mode.
	uint8_t Seconds;		///< Seconds. 0 - 59.
	uint8_t Minutes;		///< Minutes. 0 - 59.
	uint8_t Hours;			///< Hours. 0 - 59.
	uint8_t Type;			///< 0 = Film (24fps) , 1 = EBU (25fps), 2 = DF (29.97fps), 3 = SMPTE (30fps)
} __attribute__((packed));
}  // namespace artnet

class ArtNetTimeCode {
public:
	virtual ~ArtNetTimeCode() = default;
	virtual void Handler(const struct artnet::TimeCode *)= 0;
};

#endif /* ARTNETTIMECODE_H_ */
