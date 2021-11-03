/**
 * @file dmx_config.h
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

#ifndef LINUX_DMX_CONFIG_H_
#define LINUX_DMX_CONFIG_H_

namespace dmxsingle {
namespace max {
static constexpr auto OUT = 1U;
static constexpr auto IN = 1U;
}  // namespace max
}  // namespace dmxsingle

namespace dmxmulti {
namespace max {
static constexpr auto OUT = 4U;
static constexpr auto IN = 4U;
}  // namespace max
}  // namespace dmxmulti

namespace dmx {
namespace buffer {
static constexpr auto SIZE = 516;
}  // namespace buffer
}  // namespace dmx

#endif /* LINUX_DMX_CONFIG_H_ */
