/**
 * @file serialspi.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "serial/serial.h"
#include "hal_spi.h"
 #include "firmware/debug/debug_debug.h"

void Serial::SetSpiSpeedHz(uint32_t speed_hz)
{
    DEBUG_PRINTF("speed_hz=%d", speed_hz);

    if (speed_hz == 0)
    {
        return;
    }

    spi_configuration_.speed_hz = speed_hz;
}

void Serial::SetSpiMode(uint32_t mode)
{
    DEBUG_PRINTF("mode=%d", mode);

    if (mode > 3)
    {
        return;
    }

    spi_configuration_.mode = static_cast<uint8_t>(mode);
}

bool Serial::InitSpi()
{
    DEBUG_ENTRY();

    FUNC_PREFIX(SpiBegin());
    FUNC_PREFIX(SpiSetSpeedHz(spi_configuration_.speed_hz));
    FUNC_PREFIX(SpiChipSelect(SPI_CS0));
    FUNC_PREFIX(SpiSetDataMode(spi_configuration_.mode));

    DEBUG_EXIT();
    return true;
}

void Serial::SendSpi(const uint8_t* data, uint32_t length)
{
    DEBUG_ENTRY();

    FUNC_PREFIX(SpiWritenb(reinterpret_cast<const char*>(data), length));

    DEBUG_EXIT();
}
