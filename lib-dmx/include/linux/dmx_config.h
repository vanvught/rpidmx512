/**
 * @file dmx_config.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LINUX_DMX_CONFIG_H_
#define LINUX_DMX_CONFIG_H_

#include <cstdint>

#if !defined (OUTPUT_DMX_SEND_MULTI)
# define DMX_MAX_PORTS  1
#else
# if !defined(LIGHTSET_PORTS)
#  define DMX_MAX_PORTS 4
# else
#  define DMX_MAX_PORTS LIGHTSET_PORTS
# endif
#endif



namespace dmx::config::max {
	static const uint32_t PORTS = DMX_MAX_PORTS;
} // namespace dmx::config::max




namespace dmx::buffer {
static constexpr auto SIZE = 516;
} // namespace dmx::buffer


#endif /* LINUX_DMX_CONFIG_H_ */
