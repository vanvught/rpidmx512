/**
 * @file i2cdetect.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>

#include "hal_i2c.h"

static constexpr uint32_t FIRST = 0x03;
static constexpr uint32_t LAST = 0x77;

void I2cDetect()
{
    FUNC_PREFIX(I2cBegin());
    FUNC_PREFIX(I2cSetBaudrate(100000));

    puts("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

    for (uint32_t i = 0; i < 128; i = (i + 16))
    {
        printf("%02x: ", i);
        for (uint32_t j = 0; j < 16; j++)
        {
            /* Skip unwanted addresses */
            if ((i + j < FIRST) || (i + j > LAST))
            {
                printf("   ");
                continue;
            }

            if (FUNC_PREFIX(I2cIsConnected(static_cast<uint8_t>(i + j))))
            {
                printf("%02x ", i + j);
            }
            else
            {
                printf("-- ");
            }
        }

        puts("");
    }

    FUNC_PREFIX(I2cBegin());
}

#if defined(CONFIG_ENABLE_I2C1)
void I2c1Detect()
{
    FUNC_PREFIX(I2c1Begin());
    FUNC_PREFIX(I2c1SetBaudrate(100000));

    puts("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

    for (uint32_t i = 0; i < 128; i = (i + 16))
    {
        printf("%02x: ", i);
        for (uint32_t j = 0; j < 16; j++)
        {
            /* Skip unwanted addresses */
            if ((i + j < FIRST) || (i + j > LAST))
            {
                printf("   ");
                continue;
            }

            if (FUNC_PREFIX(I2c1IsConnected(static_cast<uint8_t>(i + j))))
            {
                printf("%02x ", i + j);
            }
            else
            {
                printf("-- ");
            }
        }

        puts("");
    }

    FUNC_PREFIX(I2c1Begin());
}
#endif
