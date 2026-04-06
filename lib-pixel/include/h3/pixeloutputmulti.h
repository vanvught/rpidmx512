/**
 * @file pixeloutputmulti.h
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_PIXELOUTPUTMULTI_H_
#define H3_PIXELOUTPUTMULTI_H_

#if defined(DEBUG_PIXEL)
#if defined(NDEBUG)
#undef NDEBUG
#define _NDEBUG
#endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>

#include "pixeltype.h"
#include "h3_spi.h"
#include "h3.h"

struct JamSTAPLDisplay;

class PixelOutputMulti final
{
   public:
    PixelOutputMulti();
    ~PixelOutputMulti();

    void ApplyConfiguration();

    inline void SetColourRTZ(uint32_t port_index, uint32_t pixel_index, uint8_t colour1, uint8_t colour2, uint8_t colour3)
    {
        SetColour(port_index, pixel_index, colour1, colour2, colour3);
    }

#define BIT_SET(a, b) ((a) |= static_cast<uint8_t>((1 << (b))))
#define BIT_CLEAR(a, b) ((a) &= static_cast<uint8_t>(~(1 << (b))))

    inline void SetColourRTZ(uint32_t port_index, uint32_t pixel_index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
    {
        uint32_t local_buffer[32] __attribute__((aligned(32)));
        const auto kPixelIndex = pixel_index * pixel::single::kRgbw;

        for (uint32_t i = 0; i < 32; i++)
        {
            local_buffer[i] = kPixelDatabuffer[kPixelIndex + i];
        }

        uint32_t j = 0;

        for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1))
        {
            // GRBW
            if (mask & green)
            {
                BIT_SET(local_buffer[j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[j], port_index);
            }

            if (mask & red)
            {
                BIT_SET(local_buffer[8 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[8 + j], port_index);
            }

            if (mask & blue)
            {
                BIT_SET(local_buffer[16 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[16 + j], port_index);
            }

            if (mask & white)
            {
                BIT_SET(local_buffer[24 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[24 + j], port_index);
            }

            j++;
        }

        // Write back to m_pBuffer1
        for (uint32_t i = 0; i < 32; i++)
        {
            kPixelDatabuffer[kPixelIndex + i] = local_buffer[i];
        }
    }

    inline void SetColourWS2801(uint32_t port_index, uint32_t pixel_index, uint8_t colour1, uint8_t colour2, uint8_t colour3)
    {
        SetColour(port_index, pixel_index, colour1, colour2, colour3);
    }

    inline void SetPixel4Bytes(uint32_t port_index, uint32_t pixel_index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
    {
        uint32_t local_buffer[32] __attribute__((aligned(32)));
        const auto kPixelIndex = pixel_index * pixel::single::kRgbw;

        for (uint32_t i = 0; i < 32; i++)
        {
            local_buffer[i] = kPixelDatabuffer[kPixelIndex + i];
        }

        uint32_t j = 0;

        for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1))
        {
            // GRBW
            if (mask & green)
            {
                BIT_SET(local_buffer[j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[j], port_index);
            }

            if (mask & red)
            {
                BIT_SET(local_buffer[8 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[8 + j], port_index);
            }

            if (mask & blue)
            {
                BIT_SET(local_buffer[16 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[16 + j], port_index);
            }

            if (mask & white)
            {
                BIT_SET(local_buffer[24 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[24 + j], port_index);
            }

            j++;
        }

        // Write back to m_pBuffer1
        for (uint32_t i = 0; i < 32; i++)
        {
            kPixelDatabuffer[kPixelIndex + i] = local_buffer[i];
        }
    }

    bool IsUpdating()
    {
        return H3SpiDmaTxIsActive(); // returns TRUE while DMA operation is active
    }

    void Update();
    void Blackout();
    void FullOn();

    uint32_t GetUserData();

    void SetJamSTAPLDisplay(JamSTAPLDisplay* jamstapl_display) { jamstapl_display_ = jamstapl_display; }

    static PixelOutputMulti* Get() { return s_this; }

   private:
    uint8_t ReverseBits(uint8_t bits);
    void SetupHC595(uint8_t t0h, uint8_t t1h);
    void SetupSPI(uint32_t speed_hz);
    bool SetupCPLD();
    void SetupBuffers();

    void SetColour(uint32_t port_index, uint32_t pixel_index, uint8_t colour1, uint8_t colour2, uint8_t colour3)
    {
        uint32_t local_buffer[24] __attribute__((aligned(32)));
        const uint32_t kIndex = pixel_index * pixel::single::kRgb;

        for (uint32_t i = 0; i < 24; i++)
        {
            local_buffer[i] = kPixelDatabuffer[kIndex + i];
        }

        uint32_t j = 0;

        for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1))
        {
            if (mask & colour1)
            {
                BIT_SET(local_buffer[j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[j], port_index);
            }
            if (mask & colour2)
            {
                BIT_SET(local_buffer[8 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[8 + j], port_index);
            }
            if (mask & colour3)
            {
                BIT_SET(local_buffer[16 + j], port_index);
            }
            else
            {
                BIT_CLEAR(local_buffer[16 + j], port_index);
            }

            j++;
        }

        // Write back to m_pBuffer1
        for (uint32_t i = 0; i < 24; i++)
        {
            kPixelDatabuffer[kIndex + i] = local_buffer[i];
        }
    }

   private:
    uint32_t buffer_size_{0};

    uint8_t* const kPixelDatabuffer{reinterpret_cast<uint8_t*>(H3_SRAM_A1_BASE + 512)};
    uint8_t* dma_buffer_{nullptr};

    JamSTAPLDisplay* jamstapl_display_{nullptr};

    bool has_cpld_{false};

    static inline PixelOutputMulti* s_this;
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#if defined(_NDEBUG)
#undef _NDEBUG
#define NDEBUG
#endif

#endif  // H3_PIXELOUTPUTMULTI_H_
