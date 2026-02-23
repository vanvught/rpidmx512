/**
 * @file json_status_emac.cpp
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

#include <cstdint>

#include "network.h"

namespace json::status::net
{

static uint32_t U64ToDec(char* dst, uint64_t v)
{
    // Write digits into a temp buffer in reverse order
    char tmp[20]; // enough for 2^64-1 = 18446744073709551615
    uint32_t i = 0;
    do
    {
        uint64_t q = v / 10;
        uint32_t r = static_cast<uint32_t>(v - q * 10);
        tmp[i++] = static_cast<char>('0' + r);
        v = q;
    } while (v > 0);

    for (uint32_t j = 0; j < i; ++j)
    {
        dst[j] = tmp[i - 1 - j];
    }
    return i;
}

uint32_t Emac(char* out_buffer, uint32_t out_buffer_size)
{
     network::iface::Counters st{};
     network::iface::GetCounters(st);

    auto* p = out_buffer;
    auto* end = out_buffer + out_buffer_size;

    auto emit_str = [&](const char* s)
    {
        while (*s && p < end) *p++ = *s++;
    };
    auto emit_u64 = [&](uint64_t v) { p += U64ToDec(p, v); };

    emit_str("{\"rx_ok\":");
    emit_u64(st.rx_ok);
    emit_str(",\"rx_err\":");
    emit_u64(st.rx_err);
    emit_str(",\"rx_drp\":");
    emit_u64(st.rx_drp);
    emit_str(",\"rx_ovr\":");
    emit_u64(st.rx_ovr);
    emit_str(",\"tx_ok\":");
    emit_u64(st.tx_ok);
    emit_str(",\"tx_err\":");
    emit_u64(st.tx_err);
    emit_str(",\"tx_drp\":");
    emit_u64(st.tx_drp);
    emit_str(",\"tx_ovr\":");
    emit_u64(st.tx_ovr);
    emit_str("}");

    return static_cast<uint32_t>(p - out_buffer);
}
} // namespace json::status::net
