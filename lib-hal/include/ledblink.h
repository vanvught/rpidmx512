/**
 * @file ledblink.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LEDBLINK_H
#define LEDBLINK_H

#include <cstdint>

namespace ledblink {
enum class Mode {
	OFF_OFF, OFF_ON, NORMAL, DATA, FAST, UNKNOWN
};
}  // namespace ledblink

class LedBlinkDisplay {
public:
	virtual ~LedBlinkDisplay() {
	}

	virtual void Print(uint32_t nState)=0;
};

#if defined (BARE_METAL)
# if defined (H3)
#  include "h3/ledblink.h"
# elif defined (GD32)
#  include "gd32/ledblink.h"
# else
#  include "rpi/ledblink.h"
# endif
#else
# include "linux/ledblink.h"
#endif

#endif /* LEDBLINK_H */
