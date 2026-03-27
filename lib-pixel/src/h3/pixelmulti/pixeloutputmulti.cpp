/**
 * @file pixeloutputmulti.cpp
 *
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

#if defined(DEBUG_PIXEL)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#pragma GCC optimize("-funroll-loops")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "pixeloutputmulti.h"
#include "pixelconfiguration.h"
#include "pixeltype.h"
#include "h3_gpio.h"
#include "h3_spi.h"
#include "jamstapl.h"
#include "irq_timer.h"
#include "h3_dma_memcpy32.h"
#include "logic_analyzer.h"
 #include "firmware/debug/debug_debug.h"

static volatile uint32_t sv_nUpdatesPerSecond;
static volatile uint32_t sv_nUpdatesPrevious;
static volatile uint32_t sv_nUpdates;

static void arm_timer_handler()
{
    sv_nUpdatesPerSecond = sv_nUpdates - sv_nUpdatesPrevious;
    sv_nUpdatesPrevious = sv_nUpdates;
}

inline uint8_t PixelOutputMulti::ReverseBits(uint8_t bits)
{
    const uint32_t kInput = bits;
    uint32_t output;
    asm("rbit %0, %1" : "=r"(output) : "r"(kInput));
    return static_cast<uint8_t>((output >> 24));
}

void PixelOutputMulti::Update()
{
    do
    { // https://github.com/vanvught/rpidmx512/issues/281
        __ISB();
    } while (H3SpiDmaTxIsActive());

    logic_analyzer::Ch2Set();

    dma::memcpy32(dma_buffer_, kPixelDatabuffer, buffer_size_);

    while (dma::memcpy32_is_active());

    logic_analyzer::Ch2Clear();

    H3SpiDmaTxStart(dma_buffer_, buffer_size_);

    sv_nUpdates = sv_nUpdates + 1;
}

void PixelOutputMulti::Blackout()
{
    DEBUG_ENTRY();

    auto& pixel_configuration = PixelConfiguration::Get();
    const auto kType = pixel_configuration.GetType();
    const auto kCount = pixel_configuration.GetCount();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        for (uint32_t port_index = 0; port_index < 8; port_index++)
        {
            SetPixel4Bytes(port_index, 0, 0, 0, 0, 0);

            for (uint32_t nPixelIndex = 1; nPixelIndex <= kCount; nPixelIndex++)
            {
                SetPixel4Bytes(port_index, nPixelIndex, 0, 0xE0, 0, 0);
            }

            if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822))
            {
                SetPixel4Bytes(port_index, 1U + kCount, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            else
            {
                SetPixel4Bytes(port_index, 1U + kCount, 0, 0, 0, 0);
            }
        }
    }
    else
    {
        memset(kPixelDatabuffer, 0, buffer_size_);
    }

    // Can be called any time.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    Update();

    // May not be interrupted.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    DEBUG_EXIT();
}

void PixelOutputMulti::FullOn()
{
    DEBUG_ENTRY();

    auto& pixel_configuration = PixelConfiguration::Get();

    const auto kType = pixel_configuration.GetType();
    const auto kCount = pixel_configuration.GetCount();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        for (uint32_t port_index = 0; port_index < 8; port_index++)
        {
            SetPixel4Bytes(port_index, 0, 0, 0, 0, 0);

            for (uint32_t pixel_index = 1; pixel_index <= kCount; pixel_index++)
            {
                SetPixel4Bytes(port_index, pixel_index, 0xFF, 0xE0, 0xFF, 0xFF);
            }

            if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822))
            {
                SetPixel4Bytes(port_index, 1U + kCount, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            else
            {
                SetPixel4Bytes(port_index, 1U + kCount, 0, 0, 0, 0);
            }
        }
    }
    else
    {
        memset(kPixelDatabuffer, 0xFF, buffer_size_);
    }

    // Can be called any time.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    Update();

    // May not be interrupted.
    do
    {
        asm volatile("isb" ::: "memory");
    } while (H3SpiDmaTxIsActive());

    DEBUG_EXIT();
}

uint32_t PixelOutputMulti::GetUserData()
{
    return sv_nUpdatesPerSecond;
}

void PixelOutputMulti::ApplyConfiguration()
{
	DEBUG_ENTRY();

    auto& pixel_configuration = PixelConfiguration::Get();

    pixel_configuration.Validate();
    
        if (!pixel_configuration.RefreshNeeded())
    {
		DEBUG_EXIT();
		return;
	}

    const auto kCount = pixel_configuration.GetCount();
    buffer_size_ = kCount * pixel_configuration.GetLedsPerPixel();

    const auto kType = pixel_configuration.GetType();

    if ((kType == pixel::LedType::kAPA102) || (kType == pixel::LedType::kSK9822) || (kType == pixel::LedType::kP9813))
    {
        buffer_size_ += kCount;
        buffer_size_ += 8;
    }

    buffer_size_ *= 8;

    DEBUG_PRINTF("buffer_size_=%d", buffer_size_);

    const auto kLowCode = pixel_configuration.GetLowCode();
    const auto kHighCode = pixel_configuration.GetHighCode();

    SetupHC595(ReverseBits(kLowCode), ReverseBits(kHighCode));

    if (pixel_configuration.IsRTZProtocol())
    {
        SetupSPI(pixel_configuration.GetClockSpeedHz());
    }
    else
    {
        if (has_cpld_)
        {
            SetupSPI(pixel_configuration.GetClockSpeedHz() * 6);
        }
        else
        {
            SetupSPI(pixel_configuration.GetClockSpeedHz() * 4);
        }
    }

    buffer_size_++;

    SetupBuffers();

    sv_nUpdatesPerSecond = 0;
    sv_nUpdatesPrevious = 0;
    sv_nUpdates = 0;
    
    DEBUG_EXIT();
}

#pragma GCC pop_options
#pragma GCC push_options
#pragma GCC optimize("Os")

PixelOutputMulti::PixelOutputMulti()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    has_cpld_ = SetupCPLD();

    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
    irq_handler_init();

    dma::memcpy32_init();

    printf("Board: %s\n", has_cpld_ ? "CPLD" : "74-logic");

    ApplyConfiguration();
    
    DEBUG_EXIT();
}

PixelOutputMulti::~PixelOutputMulti()
{
	Blackout();
	
    dma_buffer_ = nullptr;
    s_this = nullptr;
}

void PixelOutputMulti::SetupBuffers()
{
    DEBUG_ENTRY();

    uint32_t size;

    dma_buffer_ = const_cast<uint8_t*>(H3SpiDmaTxPrepare(&size));
    assert(dma_buffer_ != nullptr);

    memset(dma_buffer_, 0, size);

    DEBUG_PRINTF("nSize=%x, m_pBuffer1=%p, dma_buffer_=%p", size, kPixelDatabuffer, dma_buffer_);
    DEBUG_EXIT();
}

#define SPI_CS1 GPIO_EXT_26

void PixelOutputMulti::SetupHC595(uint8_t t0h, uint8_t t1h)
{
    DEBUG_ENTRY();

    t0h = static_cast<uint8_t>(t0h << 1);
    t1h = static_cast<uint8_t>(t1h << 1);

    DEBUG_PRINTF("nT0H=%.2x nT1H=%.2x", t0h, t1h);

    H3GpioFsel(SPI_CS1, GPIO_FSEL_OUTPUT);
    H3GpioSet(SPI_CS1);

    H3SpiBegin();
    H3SpiChipSelect(H3_SPI_CS_NONE);
    H3SpiSetSpeedHz(1000000);

    H3GpioClr(SPI_CS1);
    H3SpiWrite(static_cast<uint16_t>((t1h << 8) | t0h));
    H3GpioSet(SPI_CS1);

    DEBUG_EXIT();
}

void PixelOutputMulti::SetupSPI(uint32_t speed_hz)
{
    DEBUG_ENTRY();

    H3SpiBegin();
    H3SpiChipSelect(H3_SPI_CS0);
    H3SpiSetSpeedHz(speed_hz);

    DEBUG_PRINTF("nSpeedHz=%u", speed_hz);
    DEBUG_EXIT();
}

extern uint32_t PIXEL8X4_PROGRAM;

extern "C"
{
    uint32_t getPIXEL8X4_SIZE();
}

bool PixelOutputMulti::SetupCPLD()
{
    DEBUG_ENTRY();

    JamSTAPL jbc(reinterpret_cast<uint8_t*>(&PIXEL8X4_PROGRAM), getPIXEL8X4_SIZE(), true);
    jbc.SetJamSTAPLDisplay(jamstapl_display_);

    if (jbc.PrintInfo() == JBIC_SUCCESS)
    {
        if ((jbc.CheckCRC() == JBIC_SUCCESS) && (jbc.GetCRC() == 0x1D3C))
        {
            jbc.CheckIdCode();
            if (jbc.GetExitCode() == 0)
            {
                jbc.ReadUsercode();
                if ((jbc.GetExitCode() == 0) && (jbc.GetExportIntegerInt() != 0x0018ad81))
                {
                    jbc.Program();
                }
                DEBUG_EXIT();
                return true;
            }
        }
    }

    DEBUG_EXIT();
    return false;
}
