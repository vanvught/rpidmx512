/**
 * @file net_platform.h
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PLATFORM_H_
#define NET_PLATFORM_H_

#if defined(GD32)
/**
 * https://www.gd32-dmx.org/memory.html
 */
#include "gd32.h"
#if defined(GD32F207RG) || defined(GD32F4XX) || defined(GD32H7XX)
#define SECTION_NETWORK __attribute__((section(".network")))
#else
#define SECTION_NETWORK
#endif
#else
#include "h3.h"
#define SECTION_NETWORK
#include "../../src/emac/h3/emac.h"
inline void* get_tx_dma()
{
    extern struct coherent_region* p_coherent_region;
    auto desc_num = p_coherent_region->tx_currdescnum;
    auto* desc_p = &p_coherent_region->tx_chain[desc_num];
    auto data_start = static_cast<uintptr_t>(desc_p->buf_addr);
    return reinterpret_cast<void*>(data_start);
}
#endif

#endif // NET_PLATFORM_H_
