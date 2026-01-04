/**
 * @file pixeloutput.cpp
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_PIXEL)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("-funroll-loops")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cassert>

#include "pixeloutput.h"
#include "pixeltype.h"
#include "pixelconfiguration.h"
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
#include "gamma/gamma_tables.h"
#endif

void PixelOutput::SetColorWS28xx(uint32_t offset, uint8_t value)
{
    auto& pixel_configuration = PixelConfiguration::Get();
    assert(pixel_configuration.GetType() != pixel::Type::WS2801);
    assert(buffer_ != nullptr);
    assert(offset + 7 < buf_size_);

    offset += 1;

    const auto kLowCode = pixel_configuration.GetLowCode();
    const auto kHighCode = pixel_configuration.GetHighCode();

    for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1))
    {
        if (value & mask)
        {
            buffer_[offset] = kHighCode;
        }
        else
        {
            buffer_[offset] = kLowCode;
        }
        offset++;
    }
}

void PixelOutput::SetPixel(uint32_t pixel_index, uint8_t red, uint8_t green, uint8_t blue)
{
    auto& pixel_configuration = PixelConfiguration::Get();
    assert(pixel_index < pixel_configuration.GetCount());

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
    const auto* gamma_table = pixel_configuration.GetGammaTable();

    red = gamma_table[red];
    green = gamma_table[green];
    blue = gamma_table[blue];
#endif

    if (pixel_configuration.IsRTZProtocol())
    {
        const auto kOffset = pixel_index * 24U;

        SetColorWS28xx(kOffset, red);
        SetColorWS28xx(kOffset + 8, green);
        SetColorWS28xx(kOffset + 16, blue);
        return;
    }

    assert(buffer_ != nullptr);

    const auto kType = pixel_configuration.GetType();

    if (kType == pixel::Type::WS2801)
    {
        const auto kOffset = pixel_index * 3U;
        assert(kOffset + 2U < buf_size_);

        buffer_[kOffset] = red;
        buffer_[kOffset + 1] = green;
        buffer_[kOffset + 2] = blue;

        return;
    }

    if ((kType == pixel::Type::APA102) || (kType == pixel::Type::SK9822))
    {
        const auto kOffset = 4U + (pixel_index * 4U);
        assert(kOffset + 3U < buf_size_);

        buffer_[kOffset] = pixel_configuration.GetGlobalBrightness();
        buffer_[kOffset + 1] = red;
        buffer_[kOffset + 2] = green;
        buffer_[kOffset + 3] = blue;

        return;
    }

    if (kType == pixel::Type::P9813)
    {
        const auto kOffset = 4U + (pixel_index * 4U);
        assert(kOffset + 3 < buf_size_);

        const auto kFlag = static_cast<uint8_t>(0xC0 | ((~blue & 0xC0) >> 2) | ((~green & 0xC0) >> 4) | ((~red & 0xC0) >> 6));

        buffer_[kOffset] = kFlag;
        buffer_[kOffset + 1] = blue;
        buffer_[kOffset + 2] = green;
        buffer_[kOffset + 3] = red;

        return;
    }

    assert(0);
    __builtin_unreachable();
}

void PixelOutput::SetPixel(uint32_t pixel_index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    assert(pixel_index < PixelConfiguration::Get().GetCount());
    assert(PixelConfiguration::Get().GetType() == pixel::Type::SK6812W);

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
    const auto* gamma_table = PixelConfiguration::Get().GetGammaTable();

    red = gamma_table[red];
    green = gamma_table[green];
    blue = gamma_table[blue];
    white = gamma_table[white];
#endif

    const auto kOffset = pixel_index * 32U;

    SetColorWS28xx(kOffset, green);
    SetColorWS28xx(kOffset + 8, red);
    SetColorWS28xx(kOffset + 16, blue);
    SetColorWS28xx(kOffset + 24, white);
}
