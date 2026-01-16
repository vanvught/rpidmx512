/**
 * @file pixel.h
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXEL_H_
#define PIXEL_H_

#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")

#include <cstdint>
#include <cassert>

#if defined(PIXELPATTERNS_MULTI)
#include "pixeloutputmulti.h"
#else
#include "pixeloutput.h"
#endif
#include "pixeltype.h"
#include "pixelconfiguration.h"

namespace pixel
{
inline uint32_t GetColour(uint8_t red, uint8_t green, uint8_t blue)
{
    return static_cast<uint32_t>(red << 16) | static_cast<uint32_t>(green << 8) | blue;
}

inline uint32_t GetColour(uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    return static_cast<uint32_t>(white << 24) | static_cast<uint32_t>(red << 16) | static_cast<uint32_t>(green << 8) | blue;
}

struct PixelColours
{
    constexpr explicit PixelColours(uint32_t value) : value_(value) {}

    constexpr uint8_t White() const { return static_cast<uint8_t>((value_ >> 24) & 0xFF); }

    constexpr uint8_t Red() const { return static_cast<uint8_t>((value_ >> 16) & 0xFF); }

    constexpr uint8_t Green() const { return static_cast<uint8_t>((value_ >> 8) & 0xFF); }

    constexpr uint8_t Blue() const { return static_cast<uint8_t>(value_ & 0xFF); }

    constexpr uint32_t Raw() const { return value_; }

   private:
    uint32_t value_;
};

inline void SetPixelColour([[maybe_unused]] uint32_t port_index, uint32_t pixel_index, uint32_t colour)
{
    auto* output_type = PixelOutputType::Get();
    assert(output_type != nullptr);

    const pixel::PixelColours kColours(colour);

#if defined(PIXELPATTERNS_MULTI)
    switch (PixelConfiguration::Get().GetType())
    {
        case pixel::Type::WS2801:
            output_type->SetColourWS2801(port_index, pixel_index, kColours.Red(), kColours.Green(), kColours.Blue());
            break;

        case pixel::Type::APA102:
        case pixel::Type::SK9822:
            output_type->SetPixel4Bytes(port_index, pixel_index, 0xFF, kColours.Red(), kColours.Green(), kColours.Blue());
            break;

        case pixel::Type::P9813:
        {
            const auto kRed = kColours.Red();
            const auto kBlue = kColours.Blue();
            const uint8_t kFlag = static_cast<uint8_t>(0xC0 | ((~kBlue & 0xC0) >> 2) | ((~kRed & 0xC0) >> 4) | ((~kRed & 0xC0) >> 6));
            output_type->SetPixel4Bytes(port_index, pixel_index, kFlag, kBlue, kColours.Green(), kRed);
            break;
        }

        case pixel::Type::SK6812W:
            output_type->SetColourRTZ(port_index, pixel_index, kColours.Red(), kColours.Green(), kColours.Blue(), kColours.White());
            break;

        default:
            output_type->SetColourRTZ(port_index, pixel_index, kColours.Red(), kColours.Green(), kColours.Blue());
            break;
    }
#else // !PIXELPATTERNS_MULTI
    auto& pixel_configuration = PixelConfiguration::Get();
    const auto type = pixel_configuration.GetType();

    if (type != pixel::Type::SK6812W)
    {
        output_type->SetPixel(pixel_index, kColours.Red(), kColours.Green(), kColours.Blue());
    }
    else
    {
        if ((kColours.Red() == kColours.Green()) && (kColours.Green() == kColours.Blue()))
        {
            output_type->SetPixel(pixel_index, 0x00, 0x00, 0x00, kColours.Red());
        }
        else
        {
            output_type->SetPixel(pixel_index, kColours.Red(), kColours.Green(), kColours.Blue(), 0x00);
        }
    }

#endif // PIXELPATTERNS_MULTI
}

inline void SetPixelColour(uint32_t port_index, uint32_t colour)
{
    const auto kCount = PixelConfiguration::Get().GetCount();

    for (uint32_t i = 0; i < kCount; i++)
    {
        SetPixelColour(port_index, i, colour);
    }
}

inline bool IsUpdating()
{
    auto* output_type = PixelOutputType::Get();
    assert(output_type != nullptr);
    return output_type->IsUpdating();
}

inline void Update()
{
    auto* output_type = PixelOutputType::Get();
    assert(output_type != nullptr);
    return output_type->Update();
}
} // namespace pixel

#pragma GCC pop_options

#endif  // PIXEL_H_
