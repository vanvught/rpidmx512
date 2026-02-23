/**
 * @file serialuart.cpp
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
#include "hal_uart.h"
 #include "firmware/debug/debug_debug.h"

void Serial::SetUartBaud(uint32_t baud)
{
    DEBUG_PRINTF("baud=%d", baud);

    uart_configuration_.baud = baud;
}

void Serial::SetUartBits(uint32_t bits)
{
    DEBUG_PRINTF("bits=%d", bits);

    if ((bits >= 5) && (bits <= 9))
    {
        uart_configuration_.bits = static_cast<uint8_t>(bits);
    }
}

void Serial::SetUartParity(serial::uart::Parity parity)
{
    DEBUG_PRINTF("parity=%d", parity);

    switch (parity)
    {
        case serial::uart::Parity::kOdd:
            uart_configuration_.parity = common::ToValue(hal::uart::PARITY_ODD);
            break;
        case serial::uart::Parity::kEven:
            uart_configuration_.parity = common::ToValue(hal::uart::PARITY_EVEN);
            break;
        default:
            uart_configuration_.parity = common::ToValue(hal::uart::PARITY_NONE);
            break;
    }
}

void Serial::SetUartStopBits(uint32_t stop_bits)
{
    DEBUG_PRINTF("stop_bits=%d", stop_bits);

    if ((stop_bits == 1) || (stop_bits == 2))
    {
        uart_configuration_.stop_bits = static_cast<uint8_t>(stop_bits);
    }
}

bool Serial::InitUart()
{
    DEBUG_ENTRY();

    FUNC_PREFIX(UartBegin(EXT_UART_BASE, uart_configuration_.baud, uart_configuration_.bits, uart_configuration_.parity, uart_configuration_.stop_bits));

    DEBUG_EXIT();
    return true;
}

void Serial::SendUart(const uint8_t* data, uint32_t length)
{
    DEBUG_ENTRY();

    FUNC_PREFIX(UartTransmit(EXT_UART_BASE, data, length));

    DEBUG_EXIT();
}
