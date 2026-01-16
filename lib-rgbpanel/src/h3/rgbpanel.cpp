/**
 * @file rgbpanel.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(ORANGE_PI)
#error Orange Pi Zero only
#endif

#include <cassert>
#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "rgbpanel.h"

#include "h3_spi.h"
#include "h3_i2c.h"
#include "h3_gpio.h"
#include "board/h3_opi_zero.h"
#include "h3_cpu.h"
#include "h3_smp.h"

#include "arm/synchronize.h"

 #include "firmware/debug/debug_debug.h"

extern "C"
{
    void core1_task();
}

#define HUB75B_A GPIO_EXT_13 // PA0
#define HUB75B_B GPIO_EXT_11 // PA1
#define HUB75B_C GPIO_EXT_22 // PA2
#define HUB75B_D GPIO_EXT_15 // PA3

#define HUB75B_CK GPIO_EXT_26 // PA10
#define HUB75B_LA GPIO_EXT_7  // PA6
#define HUB75B_OE GPIO_EXT_12 // PA7

#define HUB75B_R1 GPIO_EXT_24 // PA13
#define HUB75B_G1 GPIO_EXT_23 // PA14
#define HUB75B_B1 GPIO_EXT_19 // PA15

#define HUB75B_R2 GPIO_EXT_21 // PA16
#define HUB75B_G2 GPIO_EXT_18 // PA18
#define HUB75B_B2 GPIO_EXT_16 // PA19

static uint32_t s_nColumns __attribute__((aligned(64)));
static uint32_t s_nRows;
static uint32_t s_nBufferSize;
static uint32_t s_nShowCounter;
//
static volatile bool s_bDoSwap;
static volatile uint32_t s_nUpdatesCounter;
//
static uint32_t* s_pFramebuffer1;
static uint32_t* s_pFramebuffer2;
static uint8_t* s_pTablePWM;
//
static bool s_is_core_running;

using namespace rgbpanel;

void RgbPanel::PlatformInit()
{
    h3_cpu_off(H3_CPU2);
    h3_cpu_off(H3_CPU3);

    s_nColumns = columns_;
    s_nRows = rows_;
    s_nShowCounter = 0;
    s_bDoSwap = false;
    s_nUpdatesCounter = 0;
    s_is_core_running = false;

    H3SpiEnd();

    H3GpioFsel(HUB75B_CK, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_LA, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_OE, GPIO_FSEL_OUTPUT);

    H3GpioClr(HUB75B_CK);
    H3GpioClr(HUB75B_LA);
    H3GpioSet(HUB75B_OE);

    H3GpioFsel(HUB75B_A, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_B, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_C, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_D, GPIO_FSEL_OUTPUT);

    H3GpioClr(HUB75B_A);
    H3GpioClr(HUB75B_B);
    H3GpioClr(HUB75B_C);
    H3GpioClr(HUB75B_D);

    H3GpioFsel(HUB75B_R1, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_G1, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_B1, GPIO_FSEL_OUTPUT);

    H3GpioClr(HUB75B_R1);
    H3GpioClr(HUB75B_G1);
    H3GpioClr(HUB75B_B1);

    H3GpioFsel(HUB75B_R2, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_G2, GPIO_FSEL_OUTPUT);
    H3GpioFsel(HUB75B_B2, GPIO_FSEL_OUTPUT);

    H3GpioClr(HUB75B_R2);
    H3GpioClr(HUB75B_G2);
    H3GpioClr(HUB75B_B2);

    s_nBufferSize = columns_ * rows_ * kPwmWidth;
    DEBUG_PRINTF("nBufferSize=%u", s_nBufferSize);

    s_pFramebuffer1 = new uint32_t[s_nBufferSize];
    assert(s_pFramebuffer1 != nullptr);

    s_pFramebuffer2 = new uint32_t[s_nBufferSize];
    assert(s_pFramebuffer2 != nullptr);

    DEBUG_PRINTF("%p %p", s_pFramebuffer1, s_pFramebuffer2);

    for (uint32_t i = 0; i < s_nBufferSize; i++)
    {
        s_pFramebuffer1[i] = 0;
        s_pFramebuffer2[i] = 0;
    }

    s_pTablePWM = new uint8_t[256];
    assert(s_pTablePWM != nullptr);

    for (uint32_t i = 0; i < 256; i++)
    {
        s_pTablePWM[i] = static_cast<uint8_t>((i * kPwmWidth) / 255);
    }
}

void RgbPanel::PlatformCleanUp()
{
    delete[] s_pFramebuffer1;
    delete[] s_pFramebuffer2;
    delete[] s_pTablePWM;
}

void RgbPanel::Start()
{
    if (started_)
    {
        return;
    }

    started_ = true;

    /**
     * Currently it is not possible stop/starting the additional core(s)
     * We need to keep the additional core(s) running.
     * Starting an already running core can crash the system.
     */

    if (s_is_core_running)
    {
        return;
    }

    // Remaining
    H3GpioFsel(8, GPIO_FSEL_DISABLE);  // PA8
    H3GpioFsel(9, GPIO_FSEL_DISABLE);  // PA9
    H3GpioFsel(17, GPIO_FSEL_DISABLE); // PA17
    H3GpioFsel(20, GPIO_FSEL_DISABLE); // PA20
    H3GpioFsel(21, GPIO_FSEL_DISABLE); // PA21

    puts("smp_start_core(1, core1_task)");
    smp_start_core(1, core1_task);
    puts("Running");
    s_is_core_running = true;
}

void RgbPanel::Dump()
{
    for (uint32_t row = 0; row < (rows_ / 2); row++)
    {
        printf("[");
        for (uint32_t i = 0; i < columns_; i++)
        {
            const uint32_t nIndex = (row * columns_) + i;
            printf("%x ", s_pFramebuffer1[nIndex]);
        }
        puts("]");
    }
}

void RgbPanel::Cls()
{
    auto lp = reinterpret_cast<uint64_t*>(s_pFramebuffer1);
    uint32_t n = s_nBufferSize * 4;

    while ((n / 8) > 0)
    {
        *(lp++) = 0;
        n -= 8;
    }

    position_ = 0;
    line_ = 0;
}

#pragma GCC push_options
#pragma GCC optimize("O3")

uint32_t RgbPanel::GetShowCounter()
{
    __DMB();
    return s_nShowCounter;
}

uint32_t RgbPanel::GetUpdatesCounter()
{
    __DMB();
    return s_nUpdatesCounter;
}

void RgbPanel::SetPixel(uint32_t column, uint32_t row, uint8_t red, uint8_t green, uint8_t blue)
{
    if (__builtin_expect(((column >= columns_) || (row >= rows_)), 0))
    {
        return;
    }

    if (row < (rows_ / 2))
    {
        const uint32_t kBaseIndex = (row * columns_ * kPwmWidth) + column;

        for (uint32_t nPWM = 0; nPWM < kPwmWidth; nPWM++)
        {
            const uint32_t kIndex = kBaseIndex + (nPWM * columns_);

            uint32_t nValue = s_pFramebuffer1[kIndex];

            nValue &= ~((1U << HUB75B_R1) | (1U << HUB75B_G1) | (1U << HUB75B_B1));

            if (s_pTablePWM[red] > nPWM)
            {
                nValue |= (1U << HUB75B_R1);
            }

            if (s_pTablePWM[green] > nPWM)
            {
                nValue |= (1U << HUB75B_G1);
            }

            if (s_pTablePWM[blue] > nPWM)
            {
                nValue |= (1U << HUB75B_B1);
            }

            s_pFramebuffer1[kIndex] = nValue;
        }
    }
    else
    {
        const uint32_t nBaseIndex = ((row - (rows_ / 2U)) * columns_ * kPwmWidth) + column;

        for (uint32_t nPWM = 0; nPWM < kPwmWidth; nPWM++)
        {
            const uint32_t nIndex = nBaseIndex + (nPWM * columns_);

            uint32_t nValue = s_pFramebuffer1[nIndex];
            nValue &= ~((1U << HUB75B_R2) | (1U << HUB75B_G2) | (1U << HUB75B_B2));

            if (s_pTablePWM[red] > nPWM)
            {
                nValue |= (1U << HUB75B_R2);
            }

            if (s_pTablePWM[green] > nPWM)
            {
                nValue |= (1U << HUB75B_G2);
            }

            if (s_pTablePWM[blue] > nPWM)
            {
                nValue |= (1U << HUB75B_B2);
            }

            s_pFramebuffer1[nIndex] = nValue;
        }
    }
}

void RgbPanel::Show()
{
    do
    {
        __DMB();
    } while (s_bDoSwap);

    s_bDoSwap = true;
    s_nShowCounter++;
}

void core1_task()
{
    const uint32_t kMultiplier = s_nColumns * kPwmWidth;

    uint32_t nGPIO =
        H3_PIO_PORTA->DAT & ~((1U << HUB75B_R1) | (1U << HUB75B_G1) | (1U << HUB75B_B1) | (1U << HUB75B_R2) | (1U << HUB75B_G2) | (1U << HUB75B_B2));

    for (;;)
    {
        for (uint32_t row = 0; row < (s_nRows / 2); row++)
        {
            const uint32_t nBaseIndex = row * kMultiplier;

            for (uint32_t nPWM = 0; nPWM < kPwmWidth; nPWM++)
            {
                uint32_t nIndex = nBaseIndex + (nPWM * s_nColumns);

                /* Shift in next data */
                for (uint32_t i = 0; i < s_nColumns; i++)
                {
                    const uint32_t nValue = s_pFramebuffer2[nIndex++];
                    // Clock high with data
                    H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_CK) | nValue;
                    // Clock low
                    H3_PIO_PORTA->DAT = nGPIO | nValue;
                }

                /* Blank the display */
                H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_OE);

                /* Latch the previous data */
                H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_LA) | (1U << HUB75B_OE);
                nGPIO |= (1U << HUB75B_OE);
                H3_PIO_PORTA->DAT = nGPIO;

                /* Update the row select */
                nGPIO &= ~(0xFU);
                nGPIO |= row;
                H3_PIO_PORTA->DAT = nGPIO;

                /* Enable the display */
                nGPIO &= ~(1U << HUB75B_OE);
                H3_PIO_PORTA->DAT = nGPIO;
            }
        }

        s_nUpdatesCounter = s_nUpdatesCounter + 1;

        if (s_bDoSwap)
        {
            auto pTmp = s_pFramebuffer1;
            s_pFramebuffer1 = s_pFramebuffer2;
            s_pFramebuffer2 = pTmp;
            __DMB();
            s_bDoSwap = false;
        }
    }
}
