/**
 * @file artnetnode_internal.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETNODE_INTERNAL_H_
#define ARTNETNODE_INTERNAL_H_

enum TErrorCodes {
	ARTNET_EOK = 0,
	ARTNET_ENET = -1,							///< network error
	ARTNET_EMEM = -2,							///< memory error
	ARTNET_EARG = -3,							///< argument error
	ARTNET_ESTATE = -4,							///< state error
	ARTNET_EACTION = -5							///< invalid action
};

namespace defaults {
static constexpr auto NET_SWITCH = 0;
static constexpr auto SUBNET_SWITCH = 0;
static constexpr auto UNIVERSE = 1;
}  // namespace defaults

#endif /* ARTNETNODE_INTERNAL_H_ */
