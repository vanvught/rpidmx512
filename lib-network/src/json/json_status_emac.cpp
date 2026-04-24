/**
 * @file json_status_emac.cpp
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "network_iface.h"

namespace json::status::emac {
static uint32_t U32ToDec(char* dst, uint32_t v) {
    // Write digits into a temp buffer in reverse order
    char tmp[12]; // enough for 2^32-1 = 4.294.967.294
    uint32_t i = 0;
    do {
        const auto kQ = v / 10U;
        const auto kR = v - (kQ * 10U);
        tmp[i++] = static_cast<char>('0' + kR);
        v = kQ;
    } while (v > 0);

    for (uint32_t j = 0; j < i; ++j) {
        dst[j] = tmp[i - 1 - j];
    }
    return i;
}

uint32_t Emac(char* out_buffer, uint32_t out_buffer_size) {
    network::iface::Counters counters;
    network::iface::GetCounters(counters);

    auto* p = out_buffer;
    auto* end = out_buffer + out_buffer_size;

    auto write_string = [&](const char* s) {
        while (*s && p < end) *p++ = *s++;
    };
    auto write_u32 = [&](uint32_t v) { p += U32ToDec(p, v); };

    write_string("{\"rx_ok\":");
    write_u32(counters.rx.ok);
    write_string(",\"rx_err\":");
    write_u32(counters.rx.err);
    write_string(",\"rx_drp\":");
    write_u32(counters.rx.drp);
    write_string(",\"rx_ovr\":");
    write_u32(counters.rx.ovr);
    write_string(",\"tx_ok\":");
    write_u32(counters.tx.ok);
    write_string(",\"tx_err\":");
    write_u32(counters.tx.err);
    write_string(",\"tx_drp\":");
    write_u32(counters.tx.drp);
    write_string(",\"tx_ovr\":");
    write_u32(counters.tx.ovr);
    write_string("}");

    return static_cast<uint32_t>(p - out_buffer);
}
} // namespace json::status::emac
