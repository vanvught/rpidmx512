/**
 * @file dmxnode_utils.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODE_UTILS_H_
#define DMXNODE_UTILS_H_

#include <cstdint>

namespace json
{
template <class S> static void PortSet(uint32_t port_index, S s, uint16_t& n)
{
    uint16_t value = n; // Create a local copy
    value &= static_cast<uint16_t>(~(0x3 << (port_index * 2)));
    value |= static_cast<uint16_t>((static_cast<uint32_t>(s) & 0x3) << (port_index * 2));
    n = value; // Write back to the original field
}

template <class S> static S PortGet(uint32_t port_index, uint16_t n)
{
    return static_cast<S>((n >> (port_index * 2)) & 0x3);
}
} // namespace json

#endif  // DMXNODE_UTILS_H_
