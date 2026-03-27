/**
 * @file pixeloutput.cpp
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cassert>

#include "pixeloutput.h"
#include "pixelconfiguration.h"
#include "h3_spi.h"
 #include "firmware/debug/debug_debug.h"

PixelOutput::PixelOutput()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    H3SpiBegin();

    ApplyConfiguration();

    DEBUG_EXIT();
}

PixelOutput::~PixelOutput()
{
	DEBUG_ENTRY();
	
	Blackout();

    blackout_buffer_ = nullptr;
    buffer_ = nullptr;
    s_this = nullptr;
    
    DEBUG_EXIT();
}

void PixelOutput::ApplyConfiguration()
{
    DEBUG_ENTRY();

    auto& pixel_configuration = PixelConfiguration::Get();
    pixel_configuration.Validate();
    
    if (!pixel_configuration.RefreshNeeded())
    {
		DEBUG_EXIT();
		return;
	}

    H3SpiSetSpeedHz(pixel_configuration.GetClockSpeedHz());

    const auto kCount = pixel_configuration.GetCount();

    buf_size_ = kCount * pixel_configuration.GetLedsPerPixel();

    if (pixel_configuration.IsRTZProtocol())
    {
        buf_size_ *= 8;
        buf_size_ += 1;
    }

    const auto kType = pixel_configuration.GetType();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        buf_size_ += kCount;
        buf_size_ += 8;
    }

    SetupBuffers();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        memset(buffer_, 0, 4);

        for (uint32_t pixel_index = 0; pixel_index < kCount; pixel_index++)
        {
            SetPixel(pixel_index, 0, 0, 0);
        }

        if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822))
        {
            memset(&buffer_[buf_size_ - 4], 0xFF, 4);
        }
        else
        {
            memset(&buffer_[buf_size_ - 4], 0, 4);
        }
    }
    else
    {
        buffer_[0] = 0x00;
        memset(&buffer_[1], kType == pixel::LedType::kWS2801 ? 0 : pixel_configuration.GetLowCode(), buf_size_);
    }

    memcpy(blackout_buffer_, buffer_, buf_size_);

    pixel_configuration.RefreshNeededReset();

    DEBUG_EXIT();
}

void PixelOutput::SetupBuffers()
{
    DEBUG_ENTRY();

    uint32_t size;

    buffer_ = const_cast<uint8_t*>(H3SpiDmaTxPrepare(&size));
    assert(buffer_ != nullptr);

    const auto kSizeHalf = size / 2;

    DEBUG_PRINTF("buf_size_=%u, kSizeHalf=%u", buf_size_, kSizeHalf);

    assert(buf_size_ <= kSizeHalf);

    blackout_buffer_ = buffer_ + (kSizeHalf & static_cast<uint32_t>(~3));

    DEBUG_PRINTF("size=%u, buffer_=%p, blackout_buffer_=%p", size, buffer_, blackout_buffer_);

    DEBUG_EXIT();
}

void PixelOutput::Update()
{
    assert(!IsUpdating());
    H3SpiDmaTxStart(buffer_, buf_size_);
}

void PixelOutput::Blackout()
{
    DEBUG_ENTRY();

    // Can be called any time.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    auto* buffer = buffer_;
    buffer_ = blackout_buffer_;

    auto& pixel_configuration = PixelConfiguration::Get();

    const auto kType = pixel_configuration.GetType();
    const auto kCount = pixel_configuration.GetCount();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        memset(buffer_, 0, 4);

        for (uint32_t pixel_index = 0; pixel_index < kCount; pixel_index++)
        {
            SetPixel(pixel_index, 0, 0, 0);
        }

        if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822))
        {
            memset(&buffer_[buf_size_ - 4], 0xFF, 4);
        }
        else
        {
            memset(&buffer_[buf_size_ - 4], 0, 4);
        }
    }
    else
    {
        buffer_[0] = 0x00;
        memset(&buffer_[1], kType == pixel::LedType::kWS2801 ? 0 : pixel_configuration.GetLowCode(), buf_size_);
    }

    Update();

    // A blackout may not be interrupted.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    buffer_ = buffer;

    DEBUG_EXIT();
}

void PixelOutput::FullOn()
{
    DEBUG_ENTRY();

    // Can be called any time.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    auto& pixel_configuration = PixelConfiguration::Get();

    const auto kType = pixel_configuration.GetType();
    const auto kCount = pixel_configuration.GetCount();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        memset(buffer_, 0xFF, 4);

        for (uint32_t pixel_index = 0; pixel_index < kCount; pixel_index++)
        {
            SetPixel(pixel_index, 0xFF, 0xFF, 0xFF);
        }

        if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822))
        {
            memset(&buffer_[buf_size_ - 4], 0xFF, 4);
        }
        else
        {
            memset(&buffer_[buf_size_ - 4], 0, 4);
        }
    }
    else
    {
        buffer_[0] = 0x00;
        memset(&buffer_[1], kType == pixel::LedType::kWS2801 ? 0xFF : pixel_configuration.GetHighCode(), buf_size_);
    }

    Update();

    // May not be interrupted.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    DEBUG_EXIT();
}
