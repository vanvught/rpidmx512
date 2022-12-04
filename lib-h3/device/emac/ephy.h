/**
 * @file ephy.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef EPHY_H_
#define EPHY_H_

#define H3_EPHY_DEFAULT_VALUE	0x00058000
#define H3_EPHY_DEFAULT_MASK	0xFFFF8000
#define H3_EPHY_ADDR_SHIFT		20
#define H3_EPHY_LED_POL			(1U << 17) 	// 1: active low, 0: active high
#define H3_EPHY_SHUTDOWN		(1U << 16) 	// 1: shutdown, 0: power up
#define H3_EPHY_SELECT			(1U << 15) 	// 1: internal PHY, 0: external PHY

#endif /* EPPHY_H_ */
